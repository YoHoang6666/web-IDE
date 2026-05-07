#include "FileTreeModel.h"

namespace webide {
FileTreeModel::FileTreeModel(QObject* parent) : QFileSystemModel(parent) {
    setReadOnly(false);
}
}  // namespace webide
