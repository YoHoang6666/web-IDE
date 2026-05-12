#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

#include "database/DatabaseManager.h"
#include "editor/EditorHost.h"
#include "preview/PreviewPane.h"
#include "runtime/RuntimeManager.h"
#include "sidebar/ActivityPanelHost.h"
#include "sidebar/ActivitySidebar.h"
#include "ui/runtime/RuntimePanel.h"
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

int activityToInt(ActivityId activity) {
    return static_cast<int>(activity);
}

ActivityId intToActivity(int value) {
    switch (value) {
        case 0:
            return ActivityId::Explorer;
        case 1:
            return ActivityId::Search;
        case 2:
            return ActivityId::Git;
        case 3:
            return ActivityId::Database;
        case 4:
            return ActivityId::Runtime;
        case 5:
            return ActivityId::Preview;
        case 6:
            return ActivityId::Network;
        case 7:
            return ActivityId::DevTools;
        case 8:
            return ActivityId::Extensions;
        case 9:
            return ActivityId::Settings;
        default:
            return ActivityId::Explorer;
    }
}

QWidget* createSimplePanel(const QString& text, QWidget* parent = nullptr) {
    auto* panel = new QWidget(parent);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);
    auto* label = new QLabel(text, panel);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch();
    return panel;
}
}  // namespace

MainWindow::MainWindow(WorkspaceManager* workspaceManager,
                       EditorHost* editorHost,
                       PreviewPane* previewPane,
                       DatabaseManager* databaseManager,
                       RuntimeManager* runtimeManager,
                       QWidget* parent)
    : QMainWindow(parent),
      workspaceManager_(workspaceManager),
      databaseManager_(databaseManager),
      runtimeManager_(runtimeManager),
      explorerWidget_(new FileExplorerWidget(this)),
      editorArea_(new EditorAreaWidget(this)),
      previewWidget_(new PreviewWidget(this)),
      databaseWidget_(new DatabaseWidget(this)),
      terminalWidget_(new TerminalWidget(this)),
      networkWidget_(new NetworkWidget(this)),
      runtimePanel_(new RuntimePanel(runtimeManager_, this)),
      activitySidebar_(new ActivitySidebar(this)),
      activityPanelHost_(new ActivityPanelHost(this)),
      centralShell_(new QWidget(this)),
      sidebarSplitter_(new QSplitter(Qt::Horizontal, centralShell_)),
      previewDock_(new QDockWidget(tr("Preview"), this)),
      terminalDock_(new QDockWidget(tr("Terminal"), this)),
      settings_(new QSettings(QStringLiteral("web-IDE"), QStringLiteral("web-IDE"), this)) {
    Q_UNUSED(editorHost);
    Q_UNUSED(previewPane);
    buildShell();
}

void MainWindow::buildShell() {
    resize(1700, 1000);
    setCentralWidget(centralShell_);

    previewWidget_->setNetworkWidget(networkWidget_);

    buildSidebars();
    buildDocks();
    buildMenus();
    wireSignals();
    applyTheme();
    loadState();
    registerActivityShortcuts();
    updateWindowTitle();
    statusBar()->showMessage(tr("web-IDE ready"));
}

void MainWindow::buildSidebars() {
    auto* centralLayout = new QHBoxLayout(centralShell_);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    sidebarSplitter_->setChildrenCollapsible(false);
    sidebarSplitter_->addWidget(activitySidebar_);
    sidebarSplitter_->addWidget(activityPanelHost_);
    sidebarSplitter_->addWidget(editorArea_);
    sidebarSplitter_->setStretchFactor(0, 0);
    sidebarSplitter_->setStretchFactor(1, 0);
    sidebarSplitter_->setStretchFactor(2, 1);
    sidebarSplitter_->setSizes({48, 320, 1200});
    centralLayout->addWidget(sidebarSplitter_);

    const QList<ActivityDefinition> activities = {
        {ActivityId::Explorer, QStringLiteral("E"), tr("Explorer"), QStringLiteral("Ctrl+1")},
        {ActivityId::Search, QStringLiteral("S"), tr("Search"), QStringLiteral("Ctrl+2")},
        {ActivityId::Git, QStringLiteral("G"), tr("Git"), QStringLiteral("Ctrl+3")},
        {ActivityId::Database, QStringLiteral("D"), tr("Database"), QStringLiteral("Ctrl+4")},
        {ActivityId::Runtime, QStringLiteral("R"), tr("Runtime"), QStringLiteral("Ctrl+5")},
        {ActivityId::Preview, QStringLiteral("P"), tr("Preview"), QStringLiteral("Ctrl+6")},
        {ActivityId::Network, QStringLiteral("N"), tr("Network"), QStringLiteral("Ctrl+7")},
        {ActivityId::DevTools, QStringLiteral("T"), tr("DevTools"), QStringLiteral("Ctrl+8")},
        {ActivityId::Extensions, QStringLiteral("X"), tr("Extensions"), QStringLiteral("Ctrl+9")},
        {ActivityId::Settings, QStringLiteral("⚙"), tr("Settings"), QStringLiteral("Ctrl+0")},
    };
    activitySidebar_->setActivities(activities);

    activityPanelHost_->registerPanel(ActivityId::Explorer, tr("Explorer"), explorerWidget_);
    activityPanelHost_->registerPanel(ActivityId::Search, tr("Search"), createSimplePanel(tr("Workspace search panel")));
    activityPanelHost_->registerPanel(ActivityId::Git, tr("Git"), createSimplePanel(tr("Source control panel")));
    activityPanelHost_->registerPanel(ActivityId::Database, tr("Database"), databaseWidget_);
    activityPanelHost_->registerPanel(ActivityId::Runtime, tr("Runtime"), runtimePanel_);
    activityPanelHost_->registerPanel(ActivityId::Preview, tr("Preview"), createSimplePanel(tr("Use the right Preview dock for browser output")));
    activityPanelHost_->registerPanel(ActivityId::Network, tr("Network"), networkWidget_);
    activityPanelHost_->registerPanel(ActivityId::DevTools, tr("DevTools"), createSimplePanel(tr("Use Preview > DevTools to inspect elements")));
    activityPanelHost_->registerPanel(ActivityId::Extensions, tr("Extensions"), createSimplePanel(tr("Extensions manager is planned for a later module")));
    activityPanelHost_->registerPanel(ActivityId::Settings, tr("Settings"), createSimplePanel(tr("Global settings panel")));

    connect(activitySidebar_, &ActivitySidebar::activityTriggered, this, [this](ActivityId activity) {
        setActivityVisible(activity, true);
    });
    connect(activitySidebar_, &ActivitySidebar::collapseRequested, this, [this]() {
        setActivityVisible(activeActivity_, !isInnerSidebarVisible_);
    });
    connect(activityPanelHost_, &ActivityPanelHost::collapseRequested, this, [this]() {
        setActivityVisible(activeActivity_, false);
    });

    setActivityVisible(ActivityId::Explorer, true);
}

void MainWindow::buildDocks() {
    previewDock_->setWidget(previewWidget_);
    terminalDock_->setWidget(terminalWidget_);

    addDockWidget(Qt::RightDockWidgetArea, previewDock_);
    addDockWidget(Qt::BottomDockWidgetArea, terminalDock_);
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

    QAction* sidebarToggleAction = viewMenu->addAction(tr("Toggle Left Sidebar"));
    QAction* previewToggleAction = viewMenu->addAction(tr("Toggle Preview"));
    QAction* terminalToggleAction = viewMenu->addAction(tr("Toggle Terminal"));
    QAction* splitToggleAction = viewMenu->addAction(tr("Toggle Split Editor"));
    QAction* devToolsToggleAction = viewMenu->addAction(tr("Toggle DevTools"));

    sidebarToggleAction->setCheckable(true);
    previewToggleAction->setCheckable(true);
    terminalToggleAction->setCheckable(true);
    splitToggleAction->setCheckable(true);
    devToolsToggleAction->setCheckable(true);

    sidebarToggleAction->setChecked(true);
    previewToggleAction->setChecked(true);
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
        currentWorkspace_ = folder;
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

    connect(sidebarToggleAction, &QAction::toggled, this, [this](bool visible) { setActivityVisible(activeActivity_, visible); });
    connect(previewToggleAction, &QAction::toggled, previewDock_, &QDockWidget::setVisible);
    connect(terminalToggleAction, &QAction::toggled, terminalDock_, &QDockWidget::setVisible);

    connect(splitToggleAction, &QAction::toggled, editorArea_, &EditorAreaWidget::setSplitEditorVisible);
    connect(devToolsToggleAction, &QAction::toggled, previewWidget_, &PreviewWidget::toggleDevTools);

    connect(previewDock_, &QDockWidget::visibilityChanged, previewToggleAction, &QAction::setChecked);
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
        setActivityVisible(ActivityId::Database, true);
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
                           tr("web-IDE\nA Qt6 desktop IDE shell with dual sidebar, editor, live preview, database tools, runtime manager, terminal, and network inspector."));
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

    connect(editorArea_, &EditorAreaWidget::statusMessage, this, [this](const QString& message) { statusBar()->showMessage(message, 3000); });
    connect(previewWidget_, &PreviewWidget::statusMessage, this, [this](const QString& message) { statusBar()->showMessage(message, 3000); });
    connect(databaseWidget_, &DatabaseWidget::statusMessage, this, [this](const QString& message) { statusBar()->showMessage(message, 4000); });
    connect(terminalWidget_, &TerminalWidget::statusMessage, this, [this](const QString& message) { statusBar()->showMessage(message, 3000); });
    connect(runtimePanel_, &RuntimePanel::statusMessage, this, [this](const QString& message) { statusBar()->showMessage(message, 3000); });

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
    }
}

void MainWindow::loadState() {
    restoreGeometry(settings_->value(QStringLiteral("window/geometry")).toByteArray());
    restoreState(settings_->value(QStringLiteral("window/state")).toByteArray());

    currentWorkspace_ = settings_->value(QStringLiteral("workspace/root")).toString();
    if (!currentWorkspace_.isEmpty() && QFileInfo::exists(currentWorkspace_)) {
        explorerWidget_->openFolder(currentWorkspace_);
    }
    restoreSidebarState();
}

void MainWindow::saveStateToSettings() {
    settings_->setValue(QStringLiteral("window/geometry"), saveGeometry());
    settings_->setValue(QStringLiteral("window/state"), saveState());
    settings_->setValue(QStringLiteral("workspace/root"), currentWorkspace_);
    persistSidebarState();
}

void MainWindow::restoreSidebarState() {
    activeActivity_ = intToActivity(settings_->value(QStringLiteral("sidebar/activity"), activityToInt(ActivityId::Explorer)).toInt());
    isInnerSidebarVisible_ = settings_->value(QStringLiteral("sidebar/visible"), true).toBool();
    const int width = settings_->value(QStringLiteral("sidebar/width"), 320).toInt();
    setActivityVisible(activeActivity_, isInnerSidebarVisible_);
    if (sidebarSplitter_) {
        const QList<int> sizes = {48, width, qMax(1, width * 2)};
        sidebarSplitter_->setSizes(sizes);
    }
}

void MainWindow::persistSidebarState() {
    settings_->setValue(QStringLiteral("sidebar/activity"), activityToInt(activeActivity_));
    settings_->setValue(QStringLiteral("sidebar/visible"), isInnerSidebarVisible_);
    if (sidebarSplitter_) {
        const QList<int> sizes = sidebarSplitter_->sizes();
        if (sizes.size() > 1) {
            settings_->setValue(QStringLiteral("sidebar/width"), sizes.at(1));
        }
    }
}

void MainWindow::setActivityVisible(ActivityId activity, bool visible) {
    activeActivity_ = activity;
    isInnerSidebarVisible_ = visible;
    activitySidebar_->setActiveActivity(activity);
    activityPanelHost_->setCurrentActivity(activity);
    activityPanelHost_->setVisible(visible);

    if (activity == ActivityId::Preview) {
        previewDock_->show();
    }
    if (activity == ActivityId::DevTools) {
        previewDock_->show();
        previewWidget_->toggleDevTools(true);
    }
}

void MainWindow::registerActivityShortcuts() {
    const QList<QPair<QKeySequence, ActivityId>> mappings = {
        {QKeySequence(QStringLiteral("Ctrl+1")), ActivityId::Explorer},
        {QKeySequence(QStringLiteral("Ctrl+2")), ActivityId::Search},
        {QKeySequence(QStringLiteral("Ctrl+3")), ActivityId::Git},
        {QKeySequence(QStringLiteral("Ctrl+4")), ActivityId::Database},
        {QKeySequence(QStringLiteral("Ctrl+5")), ActivityId::Runtime},
        {QKeySequence(QStringLiteral("Ctrl+6")), ActivityId::Preview},
        {QKeySequence(QStringLiteral("Ctrl+7")), ActivityId::Network},
        {QKeySequence(QStringLiteral("Ctrl+8")), ActivityId::DevTools},
        {QKeySequence(QStringLiteral("Ctrl+9")), ActivityId::Extensions},
        {QKeySequence(QStringLiteral("Ctrl+0")), ActivityId::Settings},
    };

    for (const auto& mapping : mappings) {
        auto* shortcut = new QShortcut(mapping.first, this);
        connect(shortcut, &QShortcut::activated, this, [this, mapping]() {
            setActivityVisible(mapping.second, true);
        });
    }
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

