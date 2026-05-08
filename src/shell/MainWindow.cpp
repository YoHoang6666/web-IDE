#include "MainWindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QSplitter>
#include <QTreeView>
#include <QApplication>
#include <QPlainTextEdit>
#include <QTextEdit>

#include "DockLayoutManager.h"
#include "MenuController.h"
#include "StatusBarController.h"
#include "database/DatabaseManager.h"
#include "editor/EditorHost.h"
#include "preview/PreviewPane.h"
#include "workspace/WorkspaceManager.h"

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
    buildShell();
}

void MainWindow::buildShell() {
    setWindowTitle("web-IDE");
    resize(1700, 1000);

    // =========================
    // GLOBAL DARK THEME
    // =========================
    qApp->setStyleSheet(R"(
        QMainWindow {
            background: #1e1e1e;
        }

        QWidget {
            background: #1e1e1e;
            color: #d4d4d4;
            font-family: Segoe UI;
            font-size: 13px;
        }

        QMenuBar {
            background: #2d2d30;
            color: white;
        }

        QMenuBar::item:selected {
            background: #3e3e42;
        }

        QMenu {
            background: #252526;
            color: white;
        }

        QDockWidget {
            titlebar-close-icon: none;
            titlebar-normal-icon: none;
        }

        QDockWidget::title {
            background: #2d2d30;
            padding: 6px;
            border: none;
        }

        QTreeView {
            background: #252526;
            border: none;
        }

        QPlainTextEdit {
            background: #1e1e1e;
            color: #dcdcdc;
            border: none;
            font-family: Consolas;
            font-size: 14px;
            selection-background-color: #264f78;
        }

        QTextEdit {
            background: #1e1e1e;
            border: none;
        }

        QSplitter::handle {
            background: #333333;
        }

        QStatusBar {
            background: #007acc;
            color: white;
        }
    )");

    // =========================
    // PROJECT EXPLORER
    // =========================
    auto* projectTree = new QTreeView(this);

    explorerDock_ = new QDockWidget(tr("Explorer"), this);
    explorerDock_->setWidget(projectTree);

    addDockWidget(Qt::LeftDockWidgetArea, explorerDock_);

    // =========================
    // MAIN EDITOR AREA
    // =========================
    auto* editorSplitter = new QSplitter(Qt::Horizontal, this);

    auto* primaryEditor = new QPlainTextEdit(this);
    auto* secondaryEditor = new QPlainTextEdit(this);

    primaryEditor->setPlaceholderText("Start coding...");
    secondaryEditor->setPlaceholderText("Secondary split editor");

    primaryEditor->setPlainText(
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>web-IDE</title>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Hello World</h1>\n"
        "</body>\n"
        "</html>"
    );

    editorSplitter->addWidget(primaryEditor);
    editorSplitter->addWidget(secondaryEditor);

    editorSplitter->setStretchFactor(0, 3);
    editorSplitter->setStretchFactor(1, 2);

    setCentralWidget(editorSplitter);

    // =========================
    // LIVE PREVIEW PANEL
    // =========================
    previewDock_ = new QDockWidget(tr("Preview"), this);

    if (previewPane_) {
        previewDock_->setWidget(previewPane_);
    } else {
        auto* previewFallback = new QTextEdit(this);
        previewFallback->setReadOnly(true);
        previewFallback->setPlainText(tr("Preview is unavailable."));
        previewDock_->setWidget(previewFallback);
    }

    addDockWidget(Qt::RightDockWidgetArea, previewDock_);

    // =========================
    // DATABASE PANEL
    // =========================
    databaseDock_ = new QDockWidget(tr("Database"), this);

    auto* databaseEditor = new QTextEdit(this);

    databaseEditor->setPlainText(
        "-- SQLite Query Console\n"
        "SELECT * FROM users;"
    );

    databaseDock_->setWidget(databaseEditor);

    addDockWidget(Qt::BottomDockWidgetArea, databaseDock_);

    // =========================
    // CONSOLE PANEL
    // =========================
    consoleDock_ = new QDockWidget(tr("Console"), this);

    auto* consoleOutput = new QTextEdit(this);

    consoleOutput->setPlainText(
        "[INFO] web-IDE started successfully\n"
        "[INFO] Native editor initialized\n"
    );

    consoleOutput->setReadOnly(true);

    consoleDock_->setWidget(consoleOutput);

    addDockWidget(Qt::BottomDockWidgetArea, consoleDock_);

    // =========================
    // TABIFY LOWER PANELS
    // =========================
    tabifyDockWidget(databaseDock_, consoleDock_);

    // =========================
    // SERVICES
    // =========================
    dockLayoutManager_ = new DockLayoutManager(this);
    menuController_ = new MenuController(this);
    statusBarController_ = new StatusBarController(statusBar(), this);

    dockLayoutManager_->captureDefaultState(saveState());

    menuController_->buildMenus();

    statusBarController_->showWorkspaceMessage(
        tr("web-IDE ready")
    );

    // =========================
    // SIGNALS
    // =========================
    if (workspaceManager_) {
        connect(
            workspaceManager_,
            &WorkspaceManager::workspaceOpened,
            this,
            [this](const QString& path) {
                setWindowFilePath(path);

                statusBarController_->showWorkspaceMessage(
                    tr("Workspace opened: %1").arg(path)
                );
            }
        );
    }

    if (databaseManager_) {
        connect(
            databaseManager_,
            &DatabaseManager::databaseAttached,
            this,
            [this](const QString& path) {
                statusBarController_->showWorkspaceMessage(
                    tr("Database attached: %1").arg(path)
                );
            }
        );
    }
}

}  // namespace webide
