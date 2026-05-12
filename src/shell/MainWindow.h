#pragma once

#include <QMainWindow>

#include "sidebar/ActivityDefinitions.h"

class QDockWidget;
class QSettings;
class QSplitter;
class QWidget;

namespace webide {
class WorkspaceManager;
class EditorHost;
class PreviewPane;
class DatabaseManager;
class RuntimeManager;
class FileExplorerWidget;
class EditorAreaWidget;
class PreviewWidget;
class DatabaseWidget;
class TerminalWidget;
class NetworkWidget;
class RuntimePanel;
class ActivitySidebar;
class ActivityPanelHost;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(WorkspaceManager* workspaceManager,
               EditorHost* editorHost,
               PreviewPane* previewPane,
               DatabaseManager* databaseManager,
               RuntimeManager* runtimeManager,
               QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildShell();
    void buildDocks();
    void buildSidebars();
    void buildMenus();
    void wireSignals();
    void loadState();
    void saveStateToSettings();
    void applyTheme();
    void updateWindowTitle();
    void setActivityVisible(ActivityId activity, bool visible);
    void registerActivityShortcuts();
    void restoreSidebarState();
    void persistSidebarState();

    WorkspaceManager* workspaceManager_;
    DatabaseManager* databaseManager_;
    RuntimeManager* runtimeManager_;

    FileExplorerWidget* explorerWidget_;
    EditorAreaWidget* editorArea_;
    PreviewWidget* previewWidget_;
    DatabaseWidget* databaseWidget_;
    TerminalWidget* terminalWidget_;
    NetworkWidget* networkWidget_;
    RuntimePanel* runtimePanel_;
    ActivitySidebar* activitySidebar_;
    ActivityPanelHost* activityPanelHost_;

    QWidget* centralShell_;
    QSplitter* sidebarSplitter_;

    QDockWidget* previewDock_;
    QDockWidget* terminalDock_;

    QSettings* settings_;
    QString currentWorkspace_;
    ActivityId activeActivity_ = ActivityId::Explorer;
    bool isInnerSidebarVisible_ = true;
};
}  // namespace webide

