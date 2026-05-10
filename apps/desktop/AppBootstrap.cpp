#include "AppBootstrap.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QStringList>
#include <QSysInfo>

#include "src/core/Logger.h"
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
    configureLogging(arguments);
    logStartupDiagnostics(arguments);
    logBuildEnvironment();
    logRuntimeDiscovery();

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

void AppBootstrap::configureLogging(const QStringList& arguments) {
    Q_UNUSED(arguments);
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!basePath.isEmpty()) {
        QDir().mkpath(basePath);
        Logger::configureGlobal(QStringLiteral("desktop"), QDir(basePath).filePath(QStringLiteral("runtime.log")));
    } else {
        Logger::configureGlobal(QStringLiteral("desktop"));
    }
}

void AppBootstrap::logStartupDiagnostics(const QStringList& arguments) const {
    Logger::global().info(QStringLiteral("Startup diagnostics begin"), QStringLiteral("startup"));
    Logger::global().info(QStringLiteral("Arguments: %1").arg(arguments.join(' ')), QStringLiteral("startup"));
    Logger::global().info(QStringLiteral("Working directory: %1").arg(QDir::currentPath()), QStringLiteral("startup"));
    Logger::global().info(QStringLiteral("Qt version: %1").arg(QString::fromLatin1(qVersion())), QStringLiteral("startup"));
}

void AppBootstrap::logBuildEnvironment() const {
    Logger::global().info(QStringLiteral("Build environment: %1 (%2)")
                              .arg(QSysInfo::prettyProductName(), QSysInfo::kernelVersion()),
                          QStringLiteral("build"));
    Logger::global().info(QStringLiteral("CPU architecture: %1").arg(QSysInfo::buildCpuArchitecture()), QStringLiteral("build"));
    Logger::global().info(QStringLiteral("Qt library paths: %1").arg(QLibraryInfo::path(QLibraryInfo::LibrariesPath)),
                          QStringLiteral("build"));
#if WEBIDE_USE_SCINTILLA
    Logger::global().info(QStringLiteral("Editor backend: Scintilla enabled"), QStringLiteral("build"));
#endif
#if WEBIDE_USE_MONACO
    Logger::global().info(QStringLiteral("Editor backend: Monaco enabled"), QStringLiteral("build"));
#endif
}

void AppBootstrap::logRuntimeDiscovery() const {
    const QString cefRoot = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("cef"));
    const bool cefPresent = QFileInfo::exists(cefRoot);
    Logger::global().info(QStringLiteral("Runtime discovery: CEF assets %1").arg(cefPresent ? QStringLiteral("found") : QStringLiteral("missing")),
                          QStringLiteral("runtime"));
    Logger::global().info(QStringLiteral("Runtime discovery: App data path %1")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)),
                          QStringLiteral("runtime"));
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
