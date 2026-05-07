#pragma once

#include <QMainWindow>

class QDockWidget;

namespace webide {
class WorkspaceManager;
class EditorHost;
class PreviewPane;
class DatabaseManager;
class DockLayoutManager;
class MenuController;
class StatusBarController;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(WorkspaceManager* workspaceManager,
               EditorHost* editorHost,
               PreviewPane* previewPane,
               DatabaseManager* databaseManager,
               QWidget* parent = nullptr);

private:
    void buildShell();

    WorkspaceManager* workspaceManager_;
    EditorHost* editorHost_;
    PreviewPane* previewPane_;
    DatabaseManager* databaseManager_;
    QDockWidget* explorerDock_;
    QDockWidget* previewDock_;
    QDockWidget* databaseDock_;
    QDockWidget* consoleDock_;
    DockLayoutManager* dockLayoutManager_;
    MenuController* menuController_;
    StatusBarController* statusBarController_;
};
}  // namespace webide
