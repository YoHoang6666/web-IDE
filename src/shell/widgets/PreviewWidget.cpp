#include "PreviewWidget.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QMetaObject>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtWebEngineCore/QWebEngineUrlRequestInfo>
#include <QtWebEngineCore/QWebEngineUrlRequestInterceptor>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>

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
}  // namespace

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent),
      toolbar_(new QToolBar(this)),
      splitter_(new QSplitter(Qt::Vertical, this)),
      previewView_(new QWebEngineView(splitter_)),
      devToolsView_(new QWebEngineView(splitter_)),
      profile_(new QWebEngineProfile(this)),
      previewPage_(new QWebEnginePage(profile_, this)),
      devToolsPage_(new QWebEnginePage(profile_, this)),
      fileWatcher_(new QFileSystemWatcher(this)) {
    auto* interceptor = new RequestInterceptor(this);
    profile_->setUrlRequestInterceptor(interceptor);

    previewView_->setPage(previewPage_);
    devToolsView_->setPage(devToolsPage_);
    previewPage_->setDevToolsPage(devToolsPage_);

    setupUi();
    setupContextMenu();

    connect(fileWatcher_, &QFileSystemWatcher::fileChanged, this, [this](const QString&) {
        reloadCurrentFile();
    });

    connect(previewPage_, &QWebEnginePage::loadFinished, this, [this](bool ok) {
        emit statusMessage(ok ? tr("Preview loaded") : tr("Preview failed to load"));
    });
}

void PreviewWidget::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QAction* backAction = toolbar_->addAction(tr("Back"));
    QAction* forwardAction = toolbar_->addAction(tr("Forward"));
    QAction* refreshAction = toolbar_->addAction(tr("Refresh"));
    QAction* devToolsAction = toolbar_->addAction(tr("Open DevTools"));

    connect(backAction, &QAction::triggered, this, &PreviewWidget::goBack);
    connect(forwardAction, &QAction::triggered, this, &PreviewWidget::goForward);
    connect(refreshAction, &QAction::triggered, this, &PreviewWidget::refresh);
    connect(devToolsAction, &QAction::triggered, this, [this]() {
        toggleDevTools(true);
    });

    splitter_->addWidget(previewView_);
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
        QMenu* menu = previewPage_->createStandardContextMenu();
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
    if (auto* interceptor = qobject_cast<RequestInterceptor*>(profile_->urlRequestInterceptor())) {
        interceptor->setNetworkWidget(networkWidget);
    }
}

void PreviewWidget::previewFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    currentFile_ = filePath;

    const QStringList watched = fileWatcher_->files();
    if (!watched.isEmpty()) {
        fileWatcher_->removePaths(watched);
    }
    fileWatcher_->addPath(filePath);

    previewView_->load(QUrl::fromLocalFile(filePath));
}

void PreviewWidget::previewHtmlContent(const QString& html, const QString& sourcePath) {
    const QUrl baseUrl = sourcePath.isEmpty() ? QUrl() : QUrl::fromLocalFile(QFileInfo(sourcePath).absolutePath() + QLatin1Char('/'));
    previewView_->setHtml(html, baseUrl);
}

void PreviewWidget::refresh() { previewView_->reload(); }

void PreviewWidget::goBack() { previewView_->back(); }

void PreviewWidget::goForward() { previewView_->forward(); }

void PreviewWidget::toggleDevTools(bool visible) { devToolsView_->setVisible(visible); }

bool PreviewWidget::isDevToolsVisible() const { return devToolsView_->isVisible(); }

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
