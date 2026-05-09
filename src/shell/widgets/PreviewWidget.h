#pragma once

#include <QWidget>

class QFileSystemWatcher;
class QToolBar;
class QUrl;
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineView;
class QSplitter;
#include <QtWebEngineCore/QWebEngineUrlRequestInterceptor>

namespace webide {
class NetworkWidget;

class PreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);

    void setNetworkWidget(NetworkWidget* networkWidget);
    void previewFile(const QString& filePath);
    void previewHtmlContent(const QString& html, const QString& sourcePath);
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

    QWebEngineUrlRequestInterceptor* requestInterceptor_ = nullptr;
    QToolBar* toolbar_;
    QSplitter* splitter_;
    QWebEngineView* previewView_;
    QWebEngineView* devToolsView_;
    QWebEngineProfile* profile_;
    QWebEnginePage* previewPage_;
    QWebEnginePage* devToolsPage_;
    QFileSystemWatcher* fileWatcher_;
    QString currentFile_;
};
}  // namespace webide
