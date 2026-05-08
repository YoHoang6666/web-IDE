#pragma once

#include <QWidget>

class QFileSystemModel;
class QLabel;
class QModelIndex;
class QTreeView;

namespace webide {
class FileExplorerWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileExplorerWidget(QWidget* parent = nullptr);

    void openFolder(const QString& path);
    QString workspaceRoot() const;

signals:
    void fileOpenRequested(const QString& filePath);
    void workspaceChanged(const QString& path);

private:
    void setupUi();
    void setupConnections();
    void showContextMenu(const QPoint& pos);
    void createNewFile(const QModelIndex& index);
    void createNewFolder(const QModelIndex& index);
    void renameEntry(const QModelIndex& index);
    void deleteEntry(const QModelIndex& index);
    void refresh();

    QString indexPath(const QModelIndex& index) const;

    QFileSystemModel* model_;
    QLabel* rootLabel_;
    QTreeView* treeView_;
    QString workspaceRoot_;
};
}  // namespace webide
