#include "AppBootstrap.h"

#include <QApplication>
#include <QStringList>

#include "src/database/DatabaseManager.h"
#include "src/database/DatabaseWatcherService.h"
#include "src/database/QueryEngine.h"
#include "src/database/SQLiteConnectionPool.h"
#include "src/editor/DocumentManager.h"
#include "src/editor/EditorHost.h"
#include "src/editor/EditorTabManager.h"
#include "src/preview/DevToolsManager.h"
#include "src/preview/LiveReloadController.h"
#include "src/preview/PreviewPane.h"
#include "src/preview/PreviewSessionManager.h"
#include "src/shell/MainWindow.h"
#include "src/sync/SyncCoordinator.h"
#include "src/workspace/FileSystemService.h"
#include "src/workspace/FileWatcherService.h"
#include "src/workspace/WorkspaceManager.h"

namespace webide {
AppBootstrap::AppBootstrap() = default;
AppBootstrap::~AppBootstrap() = default;

int AppBootstrap::run(QApplication& app, const QStringList& arguments) {
    Q_UNUSED(arguments);

    workspaceManager_ = std::make_unique<WorkspaceManager>();
    fileSystemService_ = std::make_unique<FileSystemService>();
    fileWatcherService_ = std::make_unique<FileWatcherService>();
    editorTabManager_ = std::make_unique<EditorTabManager>();
    documentManager_ = std::make_unique<DocumentManager>();
    editorHost_ = std::make_unique<EditorHost>();
    previewPane_ = std::make_unique<PreviewPane>();
    previewSessions_ = std::make_unique<PreviewSessionManager>();
    liveReloadController_ = std::make_unique<LiveReloadController>();
    devToolsManager_ = std::make_unique<DevToolsManager>();
    databaseManager_ = std::make_unique<DatabaseManager>();
    connectionPool_ = std::make_unique<SQLiteConnectionPool>();
    queryEngine_ = std::make_unique<QueryEngine>();
    databaseWatcherService_ = std::make_unique<DatabaseWatcherService>();
    syncCoordinator_ = std::make_unique<SyncCoordinator>();

    wireCoreServices();

    mainWindow_ = std::make_unique<MainWindow>(workspaceManager_.get(), editorHost_.get(), previewPane_.get(), databaseManager_.get());
    mainWindow_->show();
    return app.exec();
}

void AppBootstrap::wireCoreServices() {
    documentManager_->bindFileSystem(fileSystemService_.get());
    syncCoordinator_->attachPreview(previewPane_.get());
    syncCoordinator_->attachLiveReloadController(liveReloadController_.get());
    syncCoordinator_->attachFileWatcher(fileWatcherService_.get());
    syncCoordinator_->attachDatabaseWatcher(databaseWatcherService_.get());
    liveReloadController_->bindPreviewPane(previewPane_.get());
    devToolsManager_->bindPreviewPane(previewPane_.get());
    queryEngine_->bindConnectionPool(connectionPool_.get());
    databaseManager_->bindConnectionPool(connectionPool_.get());
}
}  // namespace webide
