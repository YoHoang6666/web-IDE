#include "DatabaseWatcherService.h"

namespace webide {
DatabaseWatcherService::DatabaseWatcherService(QObject* parent) : QObject(parent) {}

void DatabaseWatcherService::notifyDatabaseTouched(const QString& databasePath) {
    emit changeDetected(ChangeSet{ChangeOrigin::Database, ChangeKind::Modified, databasePath, {}, QDateTime::currentDateTime().toUTC()});
}
}  // namespace webide
