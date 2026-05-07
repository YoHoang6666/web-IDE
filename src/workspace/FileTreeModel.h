#pragma once

#include <QFileSystemModel>

namespace webide {
class FileTreeModel : public QFileSystemModel {
    Q_OBJECT

public:
    explicit FileTreeModel(QObject* parent = nullptr);
};
}  // namespace webide
