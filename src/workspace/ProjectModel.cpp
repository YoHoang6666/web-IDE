#include "ProjectModel.h"

#include <QStandardItem>

namespace webide {
ProjectModel::ProjectModel(QObject* parent) : QStandardItemModel(parent) {
    setHorizontalHeaderLabels({tr("Workspace")});
}

void ProjectModel::setWorkspaceRoot(const QString& path) {
    clear();
    setHorizontalHeaderLabels({tr("Workspace")});
    auto* rootItem = new QStandardItem(path);
    invisibleRootItem()->appendRow(rootItem);
}
}  // namespace webide
