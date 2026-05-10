#pragma once

#include <QMainWindow>
#include <QPointer>

class QDockWidget;
class QSettings;
class QWidget;

namespace webide {
class PanelRegistry;
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
    void registerPanels();
    void buildMenus();
    void wireSignals();
    void handlePanelReady(const QString& id, QWidget* widget);
    void loadState();
    void saveStateToSettings();
    void applyTheme();
    void updateWindowTitle();

    WorkspaceManager* workspaceManager_;
    DatabaseManager* databaseManager_;

    PanelRegistry* panelRegistry_;
    EditorAreaWidget* editorArea_;

    QPointer<FileExplorerWidget> explorerWidget_;
    QPointer<PreviewWidget> previewWidget_;
    QPointer<DatabaseWidget> databaseWidget_;
    QPointer<TerminalWidget> terminalWidget_;
    QPointer<NetworkWidget> networkWidget_;

    QSettings* settings_;
    QString currentWorkspace_;
};
}  // namespace webide
