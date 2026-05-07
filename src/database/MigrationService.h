#pragma once

#include <QObject>
#include <QStringList>

namespace webide {
class MigrationService : public QObject {
    Q_OBJECT

public:
    explicit MigrationService(QObject* parent = nullptr);
    QStringList plannedMigrations(const QString& workspacePath) const;
};
}  // namespace webide
