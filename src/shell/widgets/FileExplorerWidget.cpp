#include "FileExplorerWidget.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QTreeView>
#include <QVBoxLayout>

namespace webide {
FileExplorerWidget::FileExplorerWidget(QWidget* parent)
    : QWidget(parent), model_(new QFileSystemModel(this)), rootLabel_(new QLabel(this)), treeView_(new QTreeView(this)) {
    setupUi();
    setupConnections();
}

void FileExplorerWidget::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    rootLabel_->setText(tr("Workspace: (not opened)"));
    rootLabel_->setWordWrap(true);

    model_->setReadOnly(false);
    model_->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    treeView_->setModel(model_);
    treeView_->setHeaderHidden(false);
    treeView_->setSortingEnabled(true);
    treeView_->sortByColumn(0, Qt::AscendingOrder);
    treeView_->setDragEnabled(true);
    treeView_->setAcceptDrops(true);
    treeView_->setDropIndicatorShown(true);
    treeView_->setDragDropMode(QAbstractItemView::InternalMove);
    treeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    layout->addWidget(rootLabel_);
    layout->addWidget(treeView_);
}

void FileExplorerWidget::setupConnections() {
    connect(treeView_, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) {
            return;
        }
        const QFileInfo info(indexPath(index));
        if (info.isFile()) {
            emit fileOpenRequested(info.absoluteFilePath());
        }
    });

    connect(treeView_, &QWidget::customContextMenuRequested, this, &FileExplorerWidget::showContextMenu);
}

void FileExplorerWidget::openFolder(const QString& path) {
    if (path.isEmpty()) {
        return;
    }

    workspaceRoot_ = path;
    const QModelIndex rootIndex = model_->setRootPath(path);
    treeView_->setRootIndex(rootIndex);
    treeView_->resizeColumnToContents(0);
    rootLabel_->setText(tr("Workspace: %1").arg(path));
    emit workspaceChanged(path);
}

QString FileExplorerWidget::workspaceRoot() const { return workspaceRoot_; }

QString FileExplorerWidget::indexPath(const QModelIndex& index) const { return model_->filePath(index); }

void FileExplorerWidget::showContextMenu(const QPoint& pos) {
    const QModelIndex index = treeView_->indexAt(pos);

    QMenu menu(this);
    QAction* newFileAction = menu.addAction(tr("New File"));
    QAction* newFolderAction = menu.addAction(tr("New Folder"));
    QAction* renameAction = menu.addAction(tr("Rename"));
    QAction* deleteAction = menu.addAction(tr("Delete"));
    menu.addSeparator();
    QAction* refreshAction = menu.addAction(tr("Refresh"));

    if (!index.isValid()) {
        renameAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }

    const QAction* selected = menu.exec(treeView_->viewport()->mapToGlobal(pos));
    if (!selected) {
        return;
    }

    if (selected == newFileAction) {
        createNewFile(index);
    } else if (selected == newFolderAction) {
        createNewFolder(index);
    } else if (selected == renameAction) {
        renameEntry(index);
    } else if (selected == deleteAction) {
        deleteEntry(index);
    } else if (selected == refreshAction) {
        refresh();
    }
}

void FileExplorerWidget::createNewFile(const QModelIndex& index) {
    QString basePath = workspaceRoot_;
    if (index.isValid()) {
        const QFileInfo info(indexPath(index));
        basePath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    }
    if (basePath.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString fileName = QInputDialog::getText(this, tr("New File"), tr("File name:"), QLineEdit::Normal, QString(), &ok);
    if (!ok || fileName.trimmed().isEmpty()) {
        return;
    }

    const QString filePath = QDir(basePath).filePath(fileName.trimmed());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, tr("New File"), tr("Failed to create file: %1").arg(filePath));
        return;
    }
    file.close();
    refresh();
    emit fileOpenRequested(filePath);
}

void FileExplorerWidget::createNewFolder(const QModelIndex& index) {
    QString basePath = workspaceRoot_;
    if (index.isValid()) {
        const QFileInfo info(indexPath(index));
        basePath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    }
    if (basePath.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString folderName = QInputDialog::getText(this, tr("New Folder"), tr("Folder name:"), QLineEdit::Normal, QString(), &ok);
    if (!ok || folderName.trimmed().isEmpty()) {
        return;
    }

    if (!QDir(basePath).mkdir(folderName.trimmed())) {
        QMessageBox::warning(this, tr("New Folder"), tr("Failed to create folder in: %1").arg(basePath));
        return;
    }
    refresh();
}

void FileExplorerWidget::renameEntry(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    treeView_->edit(index);
}

void FileExplorerWidget::deleteEntry(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    const QString path = indexPath(index);
    const QFileInfo info(path);
    const auto answer = QMessageBox::question(this, tr("Delete"), tr("Delete '%1'? This cannot be undone.").arg(info.fileName()));
    if (answer != QMessageBox::Yes) {
        return;
    }

    bool ok = false;
    if (info.isDir()) {
        QDir dir(path);
        ok = dir.removeRecursively();
    } else {
        ok = QFile::remove(path);
    }

    if (!ok) {
        QMessageBox::warning(this, tr("Delete"), tr("Failed to delete: %1").arg(path));
    }
    refresh();
}

void FileExplorerWidget::refresh() {
    if (workspaceRoot_.isEmpty()) {
        return;
    }
    const QModelIndex rootIndex = model_->setRootPath(QString());
    Q_UNUSED(rootIndex);
    treeView_->setRootIndex(model_->setRootPath(workspaceRoot_));
}
}  // namespace webide
