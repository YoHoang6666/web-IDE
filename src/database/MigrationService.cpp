#include "MigrationService.h"

namespace webide {
MigrationService::MigrationService(QObject* parent) : QObject(parent) {}

QStringList MigrationService::plannedMigrations(const QString& workspacePath) const {
    return {QStringLiteral("bootstrap:%1").arg(workspacePath)};
}
}  // namespace webide
