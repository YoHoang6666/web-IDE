#include "MainWindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QSplitter>
#include <QTreeView>

#include "DockLayoutManager.h"
#include "MenuController.h"
#include "StatusBarController.h"
#include "src/database/DatabaseManager.h"
#include "src/editor/EditorHost.h"
#include "src/preview/PreviewPane.h"
#include "src/workspace/WorkspaceManager.h"

namespace webide {
MainWindow::MainWindow(WorkspaceManager* workspaceManager,
                       EditorHost* editorHost,
                       PreviewPane* previewPane,
                       DatabaseManager* databaseManager,
                       QWidget* parent)
    : QMainWindow(parent),
      workspaceManager_(workspaceManager),
      editorHost_(editorHost),
      previewPane_(previewPane),
      databaseManager_(databaseManager) {
    Q_UNUSED(workspaceManager_);
    Q_UNUSED(databaseManager_);
    buildShell();
}

void MainWindow::buildShell() {
    setWindowTitle("web-IDE");
    resize(1600, 960);

    auto* projectTree = new QTreeView(this);
    explorerDock_ = new QDockWidget(tr("Project Explorer"), this);
    explorerDock_->setWidget(projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, explorerDock_);

    auto* editorCenter = new QSplitter(Qt::Horizontal, this);
    editorCenter->addWidget(editorHost_);
    editorCenter->addWidget(new QLabel(tr("Secondary editor split"), this));
    setCentralWidget(editorCenter);

    previewDock_ = new QDockWidget(tr("Preview"), this);
    previewDock_->setWidget(previewPane_);
    addDockWidget(Qt::RightDockWidgetArea, previewDock_);

    databaseDock_ = new QDockWidget(tr("Database"), this);
    databaseDock_->setWidget(new QLabel(tr("SQLite tables and query tools"), this));
    addDockWidget(Qt::BottomDockWidgetArea, databaseDock_);

    consoleDock_ = new QDockWidget(tr("Problems / Console"), this);
    consoleDock_->setWidget(new QLabel(tr("Build output, logs, and diagnostics"), this));
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock_);

    tabifyDockWidget(databaseDock_, consoleDock_);

    dockLayoutManager_ = new DockLayoutManager(this);
    menuController_ = new MenuController(this);
    statusBarController_ = new StatusBarController(statusBar(), this);

    dockLayoutManager_->captureDefaultState(saveState());
    menuController_->buildMenus();
    statusBarController_->showWorkspaceMessage(tr("Native IDE shell ready"));
}
}  // namespace webide
