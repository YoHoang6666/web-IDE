#include "SQLiteConnectionPool.h"

namespace webide {
SQLiteConnectionPool::SQLiteConnectionPool(QObject* parent) : QObject(parent) {}

QString SQLiteConnectionPool::openConnection(const QString& databasePath) { return databasePath; }
}  // namespace webide
