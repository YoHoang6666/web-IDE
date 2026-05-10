#pragma once

#include <QWidget>

class QFileSystemWatcher;
class QToolBar;
class QUrl;
class QLabel;
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineView;
class QSplitter;
class QStackedLayout;
class QWebSocket;
#include <QtWebEngineCore/QWebEngineUrlRequestInterceptor>

namespace webide {
class NetworkWidget;
class PreviewSessionManager;

class PreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);

    void setNetworkWidget(NetworkWidget* networkWidget);
    void setSessionManager(PreviewSessionManager* sessionManager);
    void previewFile(const QString& filePath);
    void previewHtmlContent(const QString& html, const QString& sourcePath);
    void previewServer(const QUrl& url, const QUrl& webSocketUrl = {}, bool hotReloadEnabled = false, bool trustLocalhost = true);
    void refresh();
    void goBack();
    void goForward();
    void toggleDevTools(bool visible);
    bool isDevToolsVisible() const;

signals:
    void statusMessage(const QString& message);

private:
    void setupUi();
    void setupContextMenu();
    static QUrl baseUrlForSource(const QString& sourcePath);
    void reloadCurrentFile();
    void updateServerStatus(const QString& status, const QString& detail = QString());
    void showFallback(const QString& message);
    void hideFallback();
    void connectHotReload(const QUrl& socketUrl);
    static bool isLocalHost(const QUrl& url);

    QWebEngineUrlRequestInterceptor* requestInterceptor_ = nullptr;
    QToolBar* toolbar_;
    QSplitter* splitter_;
    QWidget* previewContainer_;
    QStackedLayout* previewStack_;
    QLabel* fallbackLabel_;
    QLabel* serverStatusLabel_;
    QWebEngineView* previewView_;
    QWebEngineView* devToolsView_;
    QWebEngineProfile* profile_;
    QWebEnginePage* previewPage_;
    QWebEnginePage* devToolsPage_;
    QFileSystemWatcher* fileWatcher_;
    QWebSocket* hotReloadSocket_ = nullptr;
    PreviewSessionManager* sessionManager_ = nullptr;
    QString currentFile_;
    QString currentTargetKey_;
    QUrl currentServerUrl_;
    QUrl currentSocketUrl_;
    bool trustLocalhost_ = false;
};
}  // namespace webide
