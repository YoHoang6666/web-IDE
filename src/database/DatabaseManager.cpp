#include "DatabaseManager.h"

namespace webide {
DatabaseManager::DatabaseManager(QObject* parent) : QObject(parent) {}

void DatabaseManager::bindConnectionPool(SQLiteConnectionPool* connectionPool) { connectionPool_ = connectionPool; }

void DatabaseManager::attachDatabase(const QString& path) {
    Q_UNUSED(connectionPool_);
    if (!attachedDatabases_.contains(path)) {
        attachedDatabases_.append(path);
        emit databaseAttached(path);
    }
}

QStringList DatabaseManager::attachedDatabases() const { return attachedDatabases_; }
}  // namespace webide
