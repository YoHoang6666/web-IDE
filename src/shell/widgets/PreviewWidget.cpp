#include "PreviewWidget.h"

#include <QAction>
#include <QAbstractSocket>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QLabel>
#include <QMenu>
#include <QMetaObject>
#include <QStackedLayout>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QtWebEngineCore/QWebEngineCertificateError>
#include <QtWebEngineCore/QWebEngineSettings>
#include <QtWebEngineCore/QWebEngineUrlRequestInfo>
#include <QtWebEngineCore/QWebEngineUrlRequestInterceptor>
#include <QtWebEngineCore/QWebEnginePage>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebSockets/QWebSocket>

#include "core/Logger.h"
#include "preview/PreviewSessionManager.h"
#include "NetworkWidget.h"

namespace webide {
namespace {
class RequestInterceptor final : public QWebEngineUrlRequestInterceptor {
public:
    explicit RequestInterceptor(QObject* parent = nullptr) : QWebEngineUrlRequestInterceptor(parent) {}

    void setNetworkWidget(NetworkWidget* widget) { networkWidget_ = widget; }

    void interceptRequest(QWebEngineUrlRequestInfo& info) override {
        if (!networkWidget_) {
            return;
        }
        const QString url = info.requestUrl().toString();
        const QString method = QString::fromLatin1(info.requestMethod());
        QMetaObject::invokeMethod(networkWidget_,
                                  [widget = networkWidget_, url, method]() {
                                      if (widget) {
                                          widget->addRequest(url, method, QStringLiteral("Requested"), QStringLiteral("-"));
                                      }
                                  },
                                  Qt::QueuedConnection);
    }

private:
    NetworkWidget* networkWidget_ = nullptr;
};

bool isLocalHostUrl(const QUrl& url) {
    const QString host = url.host().toLower();
    return host == QLatin1String("localhost") || host == QLatin1String("127.0.0.1") || host == QLatin1String("::1");
}

class PreviewPage final : public QWebEnginePage {
public:
    explicit PreviewPage(QWebEngineProfile* profile, QObject* parent = nullptr) : QWebEnginePage(profile, parent) {}

    void setTrustLocalhost(bool trust) { trustLocalhost_ = trust; }

protected:
    bool certificateError(const QWebEngineCertificateError& error) override {
        if (trustLocalhost_ && isLocalHostUrl(error.url())) {
            Logger::global().warning(QStringLiteral("Allowing localhost certificate error: %1")
                                         .arg(error.url().toString()),
                                     QStringLiteral("preview"));
            return true;
        }
        return QWebEnginePage::certificateError(error);
    }

private:
    bool trustLocalhost_ = false;
};
}  // namespace

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent),
      toolbar_(new QToolBar(this)),
      splitter_(new QSplitter(Qt::Vertical, this)),
      previewContainer_(new QWidget(this)),
      previewStack_(new QStackedLayout(previewContainer_)),
      fallbackLabel_(new QLabel(tr("Preview unavailable"), previewContainer_)),
      serverStatusLabel_(new QLabel(tr("Server: idle"), this)),
      previewView_(new QWebEngineView(previewContainer_)),
      devToolsView_(new QWebEngineView(splitter_)),
      profile_(new QWebEngineProfile(this)),
      previewPage_(new PreviewPage(profile_, this)),
      devToolsPage_(new QWebEnginePage(profile_, this)),
      fileWatcher_(new QFileSystemWatcher(this)) {
    requestInterceptor_ = new RequestInterceptor(this);
    profile_->setUrlRequestInterceptor(requestInterceptor_);

    previewView_->setPage(previewPage_);
    devToolsView_->setPage(devToolsPage_);
    previewPage_->setDevToolsPage(devToolsPage_);

    fallbackLabel_->setAlignment(Qt::AlignCenter);
    fallbackLabel_->setStyleSheet(QStringLiteral("color: #cccccc; background: #1f1f1f; padding: 24px;"));
    previewStack_->setContentsMargins(0, 0, 0, 0);
    previewStack_->addWidget(previewView_);
    previewStack_->addWidget(fallbackLabel_);
    previewStack_->setCurrentWidget(previewView_);

    setupUi();
    setupContextMenu();

    connect(fileWatcher_, &QFileSystemWatcher::fileChanged, this, [this](const QString&) {
        reloadCurrentFile();
    });

    connect(previewPage_, &QWebEnginePage::loadStarted, this, [this]() {
        hideFallback();
        updateServerStatus(tr("Loading"));
    });

    connect(previewPage_, &QWebEnginePage::loadFinished, this, [this](bool ok) {
        if (ok) {
            hideFallback();
            updateServerStatus(tr("Ready"));
            emit statusMessage(tr("Preview loaded"));
        } else {
            showFallback(tr("Preview failed to load."));
            updateServerStatus(tr("Failed"));
            Logger::global().warning(QStringLiteral("Preview load failed"), QStringLiteral("preview"));
            emit statusMessage(tr("Preview failed to load"));
        }
        if (sessionManager_ && !currentTargetKey_.isEmpty()) {
            sessionManager_->updateStatus(currentTargetKey_, ok ? PreviewServerStatus::Running : PreviewServerStatus::Unreachable);
        }
    });
}

void PreviewWidget::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QAction* backAction = toolbar_->addAction(tr("Back"));
    QAction* forwardAction = toolbar_->addAction(tr("Forward"));
    QAction* refreshAction = toolbar_->addAction(tr("Refresh"));
    QAction* devToolsAction = toolbar_->addAction(tr("Open DevTools"));
    toolbar_->addSeparator();
    toolbar_->addWidget(serverStatusLabel_);

    connect(backAction, &QAction::triggered, this, &PreviewWidget::goBack);
    connect(forwardAction, &QAction::triggered, this, &PreviewWidget::goForward);
    connect(refreshAction, &QAction::triggered, this, &PreviewWidget::refresh);
    connect(devToolsAction, &QAction::triggered, this, [this]() {
        toggleDevTools(true);
    });

    splitter_->addWidget(previewContainer_);
    splitter_->addWidget(devToolsView_);
    splitter_->setStretchFactor(0, 3);
    splitter_->setStretchFactor(1, 2);

    devToolsView_->setVisible(false);

    layout->addWidget(toolbar_);
    layout->addWidget(splitter_);
}

void PreviewWidget::setupContextMenu() {
    previewView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(previewView_, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu* menu = previewView_->createStandardContextMenu();
        menu->addSeparator();
        QAction* inspectAction = menu->addAction(tr("Inspect Element"));

        connect(inspectAction, &QAction::triggered, this, [this]() {
            toggleDevTools(true);
            previewPage_->triggerAction(QWebEnginePage::InspectElement);
        });

        menu->exec(previewView_->mapToGlobal(pos));
        menu->deleteLater();
    });
}

void PreviewWidget::setNetworkWidget(NetworkWidget* networkWidget) {
    auto* interceptor =
        dynamic_cast<RequestInterceptor*>(
            requestInterceptor_);

    if (interceptor) {
        interceptor->setNetworkWidget(
            networkWidget);
    }
}

void PreviewWidget::setSessionManager(PreviewSessionManager* sessionManager) {
    sessionManager_ = sessionManager;
}

void PreviewWidget::previewFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    currentFile_ = filePath;
    currentTargetKey_ = filePath;
    currentServerUrl_ = {};
    currentSocketUrl_ = {};
    trustLocalhost_ = false;
    if (hotReloadSocket_) {
        hotReloadSocket_->close();
        hotReloadSocket_->deleteLater();
        hotReloadSocket_ = nullptr;
    }

    const QStringList watched = fileWatcher_->files();
    if (!watched.isEmpty()) {
        fileWatcher_->removePaths(watched);
    }
    fileWatcher_->addPath(filePath);

    previewView_->load(QUrl::fromLocalFile(filePath));
    if (auto* page = qobject_cast<PreviewPage*>(previewPage_)) {
        page->setTrustLocalhost(false);
    }
    updateServerStatus(tr("Local file"));
    if (sessionManager_) {
        sessionManager_->assignTarget(filePath, QUrl::fromLocalFile(filePath));
    }
}

void PreviewWidget::previewHtmlContent(const QString& html, const QString& sourcePath) {
    if (!sourcePath.isEmpty()) {
        currentFile_ = sourcePath;
        currentTargetKey_ = sourcePath;
        const QStringList watched = fileWatcher_->files();
        if (!watched.isEmpty()) {
            fileWatcher_->removePaths(watched);
        }
        if (QFileInfo::exists(sourcePath)) {
            fileWatcher_->addPath(sourcePath);
        }
    } else {
        currentTargetKey_ = QStringLiteral("inline");
    }
    currentServerUrl_ = {};
    currentSocketUrl_ = {};
    trustLocalhost_ = false;
    if (hotReloadSocket_) {
        hotReloadSocket_->close();
        hotReloadSocket_->deleteLater();
        hotReloadSocket_ = nullptr;
    }
    previewView_->setHtml(html, baseUrlForSource(sourcePath));
    if (auto* page = qobject_cast<PreviewPage*>(previewPage_)) {
        page->setTrustLocalhost(false);
    }
    updateServerStatus(tr("Inline"));
    if (sessionManager_ && !currentTargetKey_.isEmpty()) {
        sessionManager_->assignTarget(currentTargetKey_, baseUrlForSource(sourcePath));
    }
}

void PreviewWidget::previewServer(const QUrl& url, const QUrl& webSocketUrl, bool hotReloadEnabled, bool trustLocalhost) {
    if (!url.isValid()) {
        showFallback(tr("Invalid preview URL"));
        updateServerStatus(tr("Invalid URL"));
        Logger::global().warning(QStringLiteral("Preview server URL invalid"), QStringLiteral("preview"));
        return;
    }
    currentFile_.clear();
    currentServerUrl_ = url;
    currentSocketUrl_ = webSocketUrl;
    trustLocalhost_ = trustLocalhost;
    currentTargetKey_ = url.toString();

    const QStringList watched = fileWatcher_->files();
    if (!watched.isEmpty()) {
        fileWatcher_->removePaths(watched);
    }

    if (auto* page = qobject_cast<PreviewPage*>(previewPage_)) {
        page->setTrustLocalhost(trustLocalhost && isLocalHost(url));
    }
    previewPage_->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, trustLocalhost && isLocalHost(url));

    updateServerStatus(tr("Connecting"));
    previewView_->load(url);

    if (hotReloadEnabled && webSocketUrl.isValid()) {
        connectHotReload(webSocketUrl);
    } else if (hotReloadSocket_) {
        hotReloadSocket_->close();
        hotReloadSocket_->deleteLater();
        hotReloadSocket_ = nullptr;
    }

    if (sessionManager_) {
        PreviewSession session;
        session.documentPath = currentTargetKey_;
        session.targetUrl = url;
        session.webSocketUrl = webSocketUrl;
        session.hotReloadEnabled = hotReloadEnabled;
        session.trustLocalhost = trustLocalhost;
        session.status = PreviewServerStatus::Starting;
        sessionManager_->assignSession(session);
    }
}

void PreviewWidget::updateServerStatus(const QString& status, const QString& detail) {
    if (!detail.isEmpty()) {
        serverStatusLabel_->setText(tr("Server: %1 (%2)").arg(status, detail));
    } else {
        serverStatusLabel_->setText(tr("Server: %1").arg(status));
    }
}

void PreviewWidget::showFallback(const QString& message) {
    fallbackLabel_->setText(message);
    previewStack_->setCurrentWidget(fallbackLabel_);
}

void PreviewWidget::hideFallback() {
    if (previewStack_->currentWidget() != previewView_) {
        previewStack_->setCurrentWidget(previewView_);
    }
}

void PreviewWidget::connectHotReload(const QUrl& socketUrl) {
    if (hotReloadSocket_) {
        hotReloadSocket_->abort();
        hotReloadSocket_->deleteLater();
        hotReloadSocket_ = nullptr;
    }
    if (!socketUrl.isValid()) {
        return;
    }

    hotReloadSocket_ = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(hotReloadSocket_, &QWebSocket::connected, this, [this]() {
        updateServerStatus(tr("Hot reload connected"));
        Logger::global().info(QStringLiteral("Hot reload websocket connected"), QStringLiteral("preview"));
    });
    connect(hotReloadSocket_, &QWebSocket::disconnected, this, [this]() {
        updateServerStatus(tr("Hot reload disconnected"));
        Logger::global().warning(QStringLiteral("Hot reload websocket disconnected"), QStringLiteral("preview"));
    });
    connect(hotReloadSocket_, &QWebSocket::textMessageReceived, this, [this](const QString& message) {
        if (message.contains(QStringLiteral("reload"), Qt::CaseInsensitive) ||
            message.contains(QStringLiteral("refresh"), Qt::CaseInsensitive)) {
            previewView_->reload();
            updateServerStatus(tr("Reloaded"));
        }
    });
    connect(hotReloadSocket_, qOverload<QAbstractSocket::SocketError>(&QWebSocket::errorOccurred), this,
            [this](QAbstractSocket::SocketError error) {
                Logger::global().warning(QStringLiteral("Hot reload websocket error: %1").arg(static_cast<int>(error)),
                                         QStringLiteral("preview"));
                updateServerStatus(tr("Hot reload error"));
            });

    hotReloadSocket_->open(socketUrl);
}

bool PreviewWidget::isLocalHost(const QUrl& url) { return isLocalHostUrl(url); }

void PreviewWidget::refresh() { previewView_->reload(); }

void PreviewWidget::goBack() { previewView_->back(); }

void PreviewWidget::goForward() { previewView_->forward(); }

void PreviewWidget::toggleDevTools(bool visible) { devToolsView_->setVisible(visible); }

bool PreviewWidget::isDevToolsVisible() const { return devToolsView_->isVisible(); }

QUrl PreviewWidget::baseUrlForSource(const QString& sourcePath) {
    if (sourcePath.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(QFileInfo(sourcePath).absolutePath() + QLatin1Char('/'));
}

void PreviewWidget::reloadCurrentFile() {
    if (currentFile_.isEmpty()) {
        return;
    }

    QFile file(currentFile_);
    if (!file.exists()) {
        return;
    }

    previewView_->load(QUrl::fromLocalFile(currentFile_));

    if (!fileWatcher_->files().contains(currentFile_)) {
        fileWatcher_->addPath(currentFile_);
    }
}
}  // namespace webide
