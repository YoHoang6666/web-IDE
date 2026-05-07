#pragma once

#include <QStandardItemModel>

namespace webide {
class ProjectModel : public QStandardItemModel {
    Q_OBJECT

public:
    explicit ProjectModel(QObject* parent = nullptr);
    void setWorkspaceRoot(const QString& path);
};
}  // namespace webide
