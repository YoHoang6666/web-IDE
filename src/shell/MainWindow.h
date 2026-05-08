#pragma once

#include <QMainWindow>

class QDockWidget;
class QSettings;

namespace webide {
class WorkspaceManager;
class EditorHost;
class PreviewPane;
class DatabaseManager;
class FileExplorerWidget;
class EditorAreaWidget;
class PreviewWidget;
class DatabaseWidget;
class TerminalWidget;
class NetworkWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(WorkspaceManager* workspaceManager,
               EditorHost* editorHost,
               PreviewPane* previewPane,
               DatabaseManager* databaseManager,
               QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildShell();
    void buildDocks();
    void buildMenus();
    void wireSignals();
    void loadState();
    void saveStateToSettings();
    void applyTheme();
    void updateWindowTitle();

    WorkspaceManager* workspaceManager_;
    EditorHost* editorHost_;
    PreviewPane* previewPane_;
    DatabaseManager* databaseManager_;

    FileExplorerWidget* explorerWidget_;
    EditorAreaWidget* editorArea_;
    PreviewWidget* previewWidget_;
    DatabaseWidget* databaseWidget_;
    TerminalWidget* terminalWidget_;
    NetworkWidget* networkWidget_;

    QDockWidget* explorerDock_;
    QDockWidget* previewDock_;
    QDockWidget* databaseDock_;
    QDockWidget* terminalDock_;
    QDockWidget* networkDock_;

    QSettings* settings_;
    QString currentWorkspace_;
};
}  // namespace webide
