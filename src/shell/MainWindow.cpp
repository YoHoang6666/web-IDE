#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QMainWindow>
#include <QObject>

#include "PanelRegistry.h"
#include "database/DatabaseManager.h"
#include "editor/EditorHost.h"
#include "preview/PreviewPane.h"
#include "core/Logger.h"
#include "widgets/DatabaseWidget.h"
#include "widgets/EditorAreaWidget.h"
#include "widgets/FileExplorerWidget.h"
#include "widgets/NetworkWidget.h"
#include "widgets/PreviewWidget.h"
#include "widgets/TerminalWidget.h"
#include "workspace/WorkspaceManager.h"

namespace webide {
namespace {
constexpr auto kThemePath = ":/webide/themes/dark.qss";
}

MainWindow::MainWindow(WorkspaceManager* workspaceManager,
                       EditorHost* editorHost,
                       PreviewPane* previewPane,
                       DatabaseManager* databaseManager,
                       QWidget* parent)
    : QMainWindow(parent),
      workspaceManager_(workspaceManager),
      databaseManager_(databaseManager),
      panelRegistry_(new PanelRegistry(this, this)),
      editorArea_(new EditorAreaWidget(this)),
      settings_(new QSettings(QStringLiteral("web-IDE"), QStringLiteral("web-IDE"), this)) {
    Q_UNUSED(editorHost);
    Q_UNUSED(previewPane);
    buildShell();
}

void MainWindow::buildShell() {
    resize(1700, 1000);
    setCentralWidget(editorArea_);
    connect(panelRegistry_, &PanelRegistry::panelReady, this, &MainWindow::handlePanelReady);
    registerPanels();
    buildMenus();
    wireSignals();
    applyTheme();
    loadState();
    updateWindowTitle();
    statusBar()->showMessage(tr("web-IDE ready"));
}

void MainWindow::registerPanels() {
    panelRegistry_->registerPanel({QStringLiteral("explorer"),
                                   tr("Explorer"),
                                   Qt::LeftDockWidgetArea,
                                   true,
                                   QStringLiteral("core"),
                                   [](QWidget* parent) { return new FileExplorerWidget(parent); }});
    panelRegistry_->registerPanel({QStringLiteral("preview"),
                                   tr("Preview"),
                                   Qt::RightDockWidgetArea,
                                   true,
                                   QStringLiteral("core"),
                                   [](QWidget* parent) { return new PreviewWidget(parent); }});
    panelRegistry_->registerPanel({QStringLiteral("database"),
                                   tr("Database"),
                                   Qt::BottomDockWidgetArea,
                                   true,
                                   QStringLiteral("core"),
                                   [](QWidget* parent) { return new DatabaseWidget(parent); }});
    panelRegistry_->registerPanel({QStringLiteral("terminal"),
                                   tr("Terminal"),
                                   Qt::BottomDockWidgetArea,
                                   true,
                                   QStringLiteral("core"),
                                   [](QWidget* parent) { return new TerminalWidget(parent); }});
    panelRegistry_->registerPanel({QStringLiteral("network"),
                                   tr("Network"),
                                   Qt::BottomDockWidgetArea,
                                   true,
                                   QStringLiteral("core"),
                                   [](QWidget* parent) { return new NetworkWidget(parent); }});

    auto* terminalDock = panelRegistry_->dockWidget(QStringLiteral("terminal"));
    auto* databaseDock = panelRegistry_->dockWidget(QStringLiteral("database"));
    auto* networkDock = panelRegistry_->dockWidget(QStringLiteral("network"));

    if (terminalDock && databaseDock) {
        tabifyDockWidget(terminalDock, databaseDock);
    }
    if (databaseDock && networkDock) {
        tabifyDockWidget(databaseDock, networkDock);
    }
    if (terminalDock) {
        terminalDock->raise();
    }
}

void MainWindow::buildMenus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction* openFolderAction = fileMenu->addAction(tr("Open Folder"));
    QAction* openFileAction = fileMenu->addAction(tr("Open File"));
    fileMenu->addSeparator();
    QAction* saveAction = fileMenu->addAction(tr("Save"));
    QAction* saveAsAction = fileMenu->addAction(tr("Save As"));
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(tr("Exit"));

    openFolderAction->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    openFileAction->setShortcut(QKeySequence::Open);
    saveAction->setShortcut(QKeySequence::Save);
    saveAsAction->setShortcut(QKeySequence::SaveAs);

    QAction* undoAction = editMenu->addAction(tr("Undo"));
    QAction* redoAction = editMenu->addAction(tr("Redo"));
    editMenu->addSeparator();
    QAction* cutAction = editMenu->addAction(tr("Cut"));
    QAction* copyAction = editMenu->addAction(tr("Copy"));
    QAction* pasteAction = editMenu->addAction(tr("Paste"));

    undoAction->setShortcut(QKeySequence::Undo);
    redoAction->setShortcut(QKeySequence::Redo);
    cutAction->setShortcut(QKeySequence::Cut);
    copyAction->setShortcut(QKeySequence::Copy);
    pasteAction->setShortcut(QKeySequence::Paste);

    QAction* explorerToggleAction = panelRegistry_->toggleAction(QStringLiteral("explorer"));
    QAction* previewToggleAction = panelRegistry_->toggleAction(QStringLiteral("preview"));
    QAction* databaseToggleAction = panelRegistry_->toggleAction(QStringLiteral("database"));
    QAction* terminalToggleAction = panelRegistry_->toggleAction(QStringLiteral("terminal"));
    QAction* networkToggleAction = panelRegistry_->toggleAction(QStringLiteral("network"));
    if (explorerToggleAction) {
        viewMenu->addAction(explorerToggleAction);
    }
    if (previewToggleAction) {
        viewMenu->addAction(previewToggleAction);
    }
    if (databaseToggleAction) {
        viewMenu->addAction(databaseToggleAction);
    }
    if (terminalToggleAction) {
        viewMenu->addAction(terminalToggleAction);
    }
    if (networkToggleAction) {
        viewMenu->addAction(networkToggleAction);
    }
    viewMenu->addSeparator();

    QAction* splitToggleAction = viewMenu->addAction(tr("Toggle Split Editor"));
    QAction* devToolsToggleAction = viewMenu->addAction(tr("Toggle DevTools"));
    splitToggleAction->setCheckable(true);
    devToolsToggleAction->setCheckable(true);
    splitToggleAction->setChecked(editorArea_->isSplitEditorVisible());
    devToolsToggleAction->setChecked(previewWidget_ ? previewWidget_->isDevToolsVisible() : false);

    QAction* openDatabaseAction = toolsMenu->addAction(tr("Open Database"));
    QAction* runBuildAction = toolsMenu->addAction(tr("Run Build"));
    QAction* startDevServerAction = toolsMenu->addAction(tr("Start Dev Server"));

    QAction* aboutAction = helpMenu->addAction(tr("About"));

    QAction* closeTabAction = new QAction(tr("Close Tab"), this);
    closeTabAction->setShortcut(QKeySequence::Close);
    addAction(closeTabAction);

    connect(openFolderAction, &QAction::triggered, this, [this]() {
        const QString folder = QFileDialog::getExistingDirectory(this, tr("Open Folder"), currentWorkspace_);
        if (folder.isEmpty()) {
            return;
        }
        if (auto* dock = panelRegistry_->dockWidget(QStringLiteral("explorer"))) {
            dock->show();
        }
        if (auto* explorer = qobject_cast<FileExplorerWidget*>(panelRegistry_->ensurePanelWidget(QStringLiteral("explorer")))) {
            explorer->openFolder(folder);
        }
    });

    connect(openFileAction, &QAction::triggered, this, [this]() {
        const QString file = QFileDialog::getOpenFileName(this, tr("Open File"), currentWorkspace_);
        if (!file.isEmpty()) {
            editorArea_->openFile(file);
        }
    });

    connect(saveAction, &QAction::triggered, editorArea_, &EditorAreaWidget::saveCurrent);
    connect(saveAsAction, &QAction::triggered, editorArea_, &EditorAreaWidget::saveCurrentAs);
    connect(closeTabAction, &QAction::triggered, editorArea_, &EditorAreaWidget::closeCurrentTab);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    connect(undoAction, &QAction::triggered, editorArea_, &EditorAreaWidget::undo);
    connect(redoAction, &QAction::triggered, editorArea_, &EditorAreaWidget::redo);
    connect(cutAction, &QAction::triggered, editorArea_, &EditorAreaWidget::cut);
    connect(copyAction, &QAction::triggered, editorArea_, &EditorAreaWidget::copy);
    connect(pasteAction, &QAction::triggered, editorArea_, &EditorAreaWidget::paste);

    connect(splitToggleAction, &QAction::toggled, editorArea_, &EditorAreaWidget::setSplitEditorVisible);
    connect(devToolsToggleAction, &QAction::toggled, this, [this](bool visible) {
        auto* preview = qobject_cast<PreviewWidget*>(panelRegistry_->ensurePanelWidget(QStringLiteral("preview")));
        if (preview) {
            preview->toggleDevTools(visible);
        }
    });

    connect(openDatabaseAction, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this,
                                                          tr("Open SQLite Database"),
                                                          currentWorkspace_,
                                                          tr("SQLite Databases (*.db *.sqlite *.sqlite3);;All Files (*.*)"));
        if (path.isEmpty()) {
            return;
        }
        if (auto* dock = panelRegistry_->dockWidget(QStringLiteral("database"))) {
            dock->show();
        }
        if (auto* database = qobject_cast<DatabaseWidget*>(panelRegistry_->ensurePanelWidget(QStringLiteral("database")))) {
            database->openDatabase(path);
        }
    });

    connect(runBuildAction, &QAction::triggered, this, [this]() {
        if (auto* dock = panelRegistry_->dockWidget(QStringLiteral("terminal"))) {
            dock->show();
        }
        if (auto* terminal = qobject_cast<TerminalWidget*>(panelRegistry_->ensurePanelWidget(QStringLiteral("terminal")))) {
            terminal->executeCommand(QStringLiteral("cmake --build build"));
        }
    });

    connect(startDevServerAction, &QAction::triggered, this, [this]() {
        if (auto* dock = panelRegistry_->dockWidget(QStringLiteral("terminal"))) {
            dock->show();
        }
        if (auto* terminal = qobject_cast<TerminalWidget*>(panelRegistry_->ensurePanelWidget(QStringLiteral("terminal")))) {
            terminal->executeCommand(QStringLiteral("npm run dev"));
        }
    });

    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this,
                           tr("About web-IDE"),
                           tr("web-IDE\nA Qt6 desktop IDE shell with explorer, editor, live preview, database tools, terminal, and network inspector."));
    });
}

void MainWindow::wireSignals() {
    connect(editorArea_, &EditorAreaWidget::statusMessage, this, [this](const QString& message) {
        statusBar()->showMessage(message, 3000);
    });

    connect(editorArea_, &EditorAreaWidget::currentFileChanged, this,
            [this](const QString& filePath, const QString& content, bool isHtml) {
                updateWindowTitle();
                if (isHtml && previewWidget_) {
                    previewWidget_->previewHtmlContent(content, filePath);
                }
            },
            Qt::UniqueConnection);

    connect(editorArea_, &EditorAreaWidget::documentContentChanged, this,
            [this](const QString& filePath, const QString& content, bool isHtml) {
                if (isHtml && previewWidget_) {
                    previewWidget_->previewHtmlContent(content, filePath);
                }
            },
            Qt::UniqueConnection);

    if (workspaceManager_) {
        connect(workspaceManager_, &WorkspaceManager::workspaceOpened, this, [this](const QString& path) {
            currentWorkspace_ = path;
            updateWindowTitle();
        });
    }
}

void MainWindow::handlePanelReady(const QString& id, QWidget* widget) {
    if (id == QLatin1String("explorer")) {
        explorerWidget_ = qobject_cast<FileExplorerWidget*>(widget);
        if (!explorerWidget_) {
            return;
        }
        connect(explorerWidget_, &FileExplorerWidget::fileOpenRequested, editorArea_, &EditorAreaWidget::openFile, Qt::UniqueConnection);
        connect(explorerWidget_, &FileExplorerWidget::workspaceChanged, this, [this](const QString& path) {
            currentWorkspace_ = path;
            if (workspaceManager_) {
                workspaceManager_->openWorkspace(path);
            }
            updateWindowTitle();
            statusBar()->showMessage(tr("Workspace opened: %1").arg(path), 4000);
        }, Qt::UniqueConnection);
        if (!currentWorkspace_.isEmpty() && QFileInfo::exists(currentWorkspace_)) {
            explorerWidget_->openFolder(currentWorkspace_);
        }
        return;
    }

    if (id == QLatin1String("preview")) {
        previewWidget_ = qobject_cast<PreviewWidget*>(widget);
        if (!previewWidget_) {
            return;
        }
        connect(previewWidget_, &PreviewWidget::statusMessage, this, [this](const QString& message) {
            statusBar()->showMessage(message, 3000);
        }, Qt::UniqueConnection);
        if (networkWidget_) {
            previewWidget_->setNetworkWidget(networkWidget_);
        }
        return;
    }

    if (id == QLatin1String("database")) {
        databaseWidget_ = qobject_cast<DatabaseWidget*>(widget);
        if (!databaseWidget_) {
            return;
        }
        connect(databaseWidget_, &DatabaseWidget::statusMessage, this, [this](const QString& message) {
            statusBar()->showMessage(message, 4000);
        }, Qt::UniqueConnection);
        connect(databaseWidget_, &DatabaseWidget::databaseOpened, this, [this](const QString& path) {
            if (databaseManager_) {
                databaseManager_->attachDatabase(path);
            }
        }, Qt::UniqueConnection);
        return;
    }

    if (id == QLatin1String("terminal")) {
        terminalWidget_ = qobject_cast<TerminalWidget*>(widget);
        if (!terminalWidget_) {
            return;
        }
        connect(terminalWidget_, &TerminalWidget::statusMessage, this, [this](const QString& message) {
            statusBar()->showMessage(message, 3000);
        }, Qt::UniqueConnection);
        return;
    }

    if (id == QLatin1String("network")) {
        networkWidget_ = qobject_cast<NetworkWidget*>(widget);
        if (!networkWidget_) {
            return;
        }
        if (previewWidget_) {
            previewWidget_->setNetworkWidget(networkWidget_);
        }
    }
}

void MainWindow::applyTheme() {
    QFile stylesheet(QString::fromLatin1(kThemePath));
    if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    } else {
        Logger::global().warning(QStringLiteral("Failed to load dark theme from resources"), QStringLiteral("shell"));
    }
}

void MainWindow::loadState() {
    restoreGeometry(settings_->value(QStringLiteral("window/geometry")).toByteArray());
    panelRegistry_->restoreState(settings_);

    currentWorkspace_ = settings_->value(QStringLiteral("workspace/root")).toString();
    if (!currentWorkspace_.isEmpty() && QFileInfo::exists(currentWorkspace_)) {
        if (auto* explorer = qobject_cast<FileExplorerWidget*>(panelRegistry_->ensurePanelWidget(QStringLiteral("explorer")))) {
            explorer->openFolder(currentWorkspace_);
        }
    }
}

void MainWindow::saveStateToSettings() {
    settings_->setValue(QStringLiteral("window/geometry"), saveGeometry());
    panelRegistry_->saveState(settings_);
    settings_->setValue(QStringLiteral("workspace/root"), currentWorkspace_);
}

void MainWindow::updateWindowTitle() {
    const QString fileName = editorArea_->currentFilePath();
    const QString workspaceName = currentWorkspace_.isEmpty() ? tr("No Workspace") : QFileInfo(currentWorkspace_).fileName();
    if (fileName.isEmpty()) {
        setWindowTitle(tr("web-IDE - %1").arg(workspaceName));
        return;
    }
    setWindowTitle(tr("%1 - web-IDE [%2]").arg(QFileInfo(fileName).fileName(), workspaceName));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveStateToSettings();
    QMainWindow::closeEvent(event);
}
}  // namespace webide
