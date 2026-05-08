#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>

#include "database/DatabaseManager.h"
#include "editor/EditorHost.h"
#include "preview/PreviewPane.h"
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
      explorerWidget_(new FileExplorerWidget(this)),
      editorArea_(new EditorAreaWidget(this)),
      previewWidget_(new PreviewWidget(this)),
      databaseWidget_(new DatabaseWidget(this)),
      terminalWidget_(new TerminalWidget(this)),
      networkWidget_(new NetworkWidget(this)),
      explorerDock_(new QDockWidget(tr("Explorer"), this)),
      previewDock_(new QDockWidget(tr("Preview"), this)),
      databaseDock_(new QDockWidget(tr("Database"), this)),
      terminalDock_(new QDockWidget(tr("Terminal"), this)),
      networkDock_(new QDockWidget(tr("Network"), this)),
      settings_(new QSettings(QStringLiteral("web-IDE"), QStringLiteral("web-IDE"), this)) {
    Q_UNUSED(editorHost);
    Q_UNUSED(previewPane);
    buildShell();
}

void MainWindow::buildShell() {
    resize(1700, 1000);
    setCentralWidget(editorArea_);

    previewWidget_->setNetworkWidget(networkWidget_);

    buildDocks();
    buildMenus();
    wireSignals();
    applyTheme();
    loadState();
    updateWindowTitle();
    statusBar()->showMessage(tr("web-IDE ready"));
}

void MainWindow::buildDocks() {
    explorerDock_->setWidget(explorerWidget_);
    previewDock_->setWidget(previewWidget_);
    databaseDock_->setWidget(databaseWidget_);
    terminalDock_->setWidget(terminalWidget_);
    networkDock_->setWidget(networkWidget_);

    addDockWidget(Qt::LeftDockWidgetArea, explorerDock_);
    addDockWidget(Qt::RightDockWidgetArea, previewDock_);
    addDockWidget(Qt::BottomDockWidgetArea, terminalDock_);
    addDockWidget(Qt::BottomDockWidgetArea, databaseDock_);
    addDockWidget(Qt::BottomDockWidgetArea, networkDock_);

    tabifyDockWidget(terminalDock_, databaseDock_);
    tabifyDockWidget(databaseDock_, networkDock_);

    terminalDock_->raise();
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

    QAction* explorerToggleAction = viewMenu->addAction(tr("Toggle Explorer"));
    QAction* previewToggleAction = viewMenu->addAction(tr("Toggle Preview"));
    QAction* databaseToggleAction = viewMenu->addAction(tr("Toggle Database"));
    QAction* terminalToggleAction = viewMenu->addAction(tr("Toggle Terminal"));
    QAction* splitToggleAction = viewMenu->addAction(tr("Toggle Split Editor"));
    QAction* devToolsToggleAction = viewMenu->addAction(tr("Toggle DevTools"));

    explorerToggleAction->setCheckable(true);
    previewToggleAction->setCheckable(true);
    databaseToggleAction->setCheckable(true);
    terminalToggleAction->setCheckable(true);
    splitToggleAction->setCheckable(true);
    devToolsToggleAction->setCheckable(true);

    explorerToggleAction->setChecked(true);
    previewToggleAction->setChecked(true);
    databaseToggleAction->setChecked(true);
    terminalToggleAction->setChecked(true);
    splitToggleAction->setChecked(editorArea_->isSplitEditorVisible());
    devToolsToggleAction->setChecked(previewWidget_->isDevToolsVisible());

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
        explorerWidget_->openFolder(folder);
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

    connect(explorerToggleAction, &QAction::toggled, explorerDock_, &QDockWidget::setVisible);
    connect(previewToggleAction, &QAction::toggled, previewDock_, &QDockWidget::setVisible);
    connect(databaseToggleAction, &QAction::toggled, databaseDock_, &QDockWidget::setVisible);
    connect(terminalToggleAction, &QAction::toggled, terminalDock_, &QDockWidget::setVisible);

    connect(splitToggleAction, &QAction::toggled, editorArea_, &EditorAreaWidget::setSplitEditorVisible);
    connect(devToolsToggleAction, &QAction::toggled, previewWidget_, &PreviewWidget::toggleDevTools);

    connect(explorerDock_, &QDockWidget::visibilityChanged, explorerToggleAction, &QAction::setChecked);
    connect(previewDock_, &QDockWidget::visibilityChanged, previewToggleAction, &QAction::setChecked);
    connect(databaseDock_, &QDockWidget::visibilityChanged, databaseToggleAction, &QAction::setChecked);
    connect(terminalDock_, &QDockWidget::visibilityChanged, terminalToggleAction, &QAction::setChecked);

    connect(openDatabaseAction, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this,
                                                          tr("Open SQLite Database"),
                                                          currentWorkspace_,
                                                          tr("SQLite Databases (*.db *.sqlite *.sqlite3);;All Files (*.*)"));
        if (path.isEmpty()) {
            return;
        }
        databaseWidget_->openDatabase(path);
        if (databaseManager_) {
            databaseManager_->attachDatabase(path);
        }
    });

    connect(runBuildAction, &QAction::triggered, this, [this]() {
        terminalDock_->show();
        terminalWidget_->executeCommand(QStringLiteral("cmake --build build"));
    });

    connect(startDevServerAction, &QAction::triggered, this, [this]() {
        terminalDock_->show();
        terminalWidget_->executeCommand(QStringLiteral("npm run dev"));
    });

    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this,
                           tr("About web-IDE"),
                           tr("web-IDE\nA Qt6 desktop IDE shell with explorer, editor, live preview, database tools, terminal, and network inspector."));
    });
}

void MainWindow::wireSignals() {
    connect(explorerWidget_, &FileExplorerWidget::fileOpenRequested, editorArea_, &EditorAreaWidget::openFile);

    connect(explorerWidget_, &FileExplorerWidget::workspaceChanged, this, [this](const QString& path) {
        currentWorkspace_ = path;
        if (workspaceManager_) {
            workspaceManager_->openWorkspace(path);
        }
        updateWindowTitle();
        statusBar()->showMessage(tr("Workspace opened: %1").arg(path), 4000);
    });

    connect(editorArea_, &EditorAreaWidget::currentFileChanged, this, [this](const QString& filePath, const QString& content, bool isHtml) {
        updateWindowTitle();
        if (isHtml) {
            previewWidget_->previewHtmlContent(content, filePath);
        }
    });

    connect(editorArea_, &EditorAreaWidget::documentContentChanged, this, [this](const QString& filePath, const QString& content, bool isHtml) {
        if (isHtml) {
            previewWidget_->previewHtmlContent(content, filePath);
        }
    });

    connect(editorArea_, &EditorAreaWidget::statusMessage, statusBar(), [this](const QString& message) {
        statusBar()->showMessage(message, 3000);
    });
    connect(previewWidget_, &PreviewWidget::statusMessage, statusBar(), [this](const QString& message) {
        statusBar()->showMessage(message, 3000);
    });
    connect(databaseWidget_, &DatabaseWidget::statusMessage, statusBar(), [this](const QString& message) {
        statusBar()->showMessage(message, 4000);
    });
    connect(terminalWidget_, &TerminalWidget::statusMessage, statusBar(), [this](const QString& message) {
        statusBar()->showMessage(message, 3000);
    });

    connect(databaseWidget_, &DatabaseWidget::databaseOpened, this, [this](const QString& path) {
        if (databaseManager_) {
            databaseManager_->attachDatabase(path);
        }
    });

    if (workspaceManager_) {
        connect(workspaceManager_, &WorkspaceManager::workspaceOpened, this, [this](const QString& path) {
            currentWorkspace_ = path;
            updateWindowTitle();
        });
    }
}

void MainWindow::applyTheme() {
    QFile stylesheet(QString::fromLatin1(kThemePath));
    if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    } else {
        qWarning() << "Failed to load dark theme from resources";
    }
}

void MainWindow::loadState() {
    restoreGeometry(settings_->value(QStringLiteral("window/geometry")).toByteArray());
    restoreState(settings_->value(QStringLiteral("window/state")).toByteArray());

    currentWorkspace_ = settings_->value(QStringLiteral("workspace/root")).toString();
    if (!currentWorkspace_.isEmpty() && QFileInfo::exists(currentWorkspace_)) {
        explorerWidget_->openFolder(currentWorkspace_);
    }
}

void MainWindow::saveStateToSettings() {
    settings_->setValue(QStringLiteral("window/geometry"), saveGeometry());
    settings_->setValue(QStringLiteral("window/state"), saveState());
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
