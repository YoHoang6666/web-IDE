#pragma once

#include <memory>
#include <QStringList>

class QApplication;

namespace webide {
class MainWindow;
class WorkspaceManager;
class FileSystemService;
class FileWatcherService;
class EditorTabManager;
class DocumentManager;
class EditorHost;
class PreviewPane;
class PreviewSessionManager;
class LiveReloadController;
class DevToolsManager;
class DatabaseManager;
class SQLiteConnectionPool;
class QueryEngine;
class DatabaseWatcherService;
class SyncCoordinator;
class RuntimeManager;

class AppBootstrap {
public:
    AppBootstrap();
    ~AppBootstrap();
    int run(QApplication& app, const QStringList& arguments);

private:
    void wireCoreServices();

    std::unique_ptr<WorkspaceManager> workspaceManager_;
    std::unique_ptr<FileSystemService> fileSystemService_;
    std::unique_ptr<FileWatcherService> fileWatcherService_;
    std::unique_ptr<EditorTabManager> editorTabManager_;
    std::unique_ptr<DocumentManager> documentManager_;
    std::unique_ptr<EditorHost> editorHost_;
    std::unique_ptr<PreviewPane> previewPane_;
    std::unique_ptr<PreviewSessionManager> previewSessions_;
    std::unique_ptr<LiveReloadController> liveReloadController_;
    std::unique_ptr<DevToolsManager> devToolsManager_;
    std::unique_ptr<DatabaseManager> databaseManager_;
    std::unique_ptr<SQLiteConnectionPool> connectionPool_;
    std::unique_ptr<QueryEngine> queryEngine_;
    std::unique_ptr<DatabaseWatcherService> databaseWatcherService_;
    std::unique_ptr<SyncCoordinator> syncCoordinator_;
    std::unique_ptr<RuntimeManager> runtimeManager_;
    std::unique_ptr<MainWindow> mainWindow_;
};
}  // namespace webide
