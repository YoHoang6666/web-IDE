#include "QueryEngine.h"

#include <QVariantMap>

#include "SQLiteConnectionPool.h"

namespace webide {
QueryEngine::QueryEngine(QObject* parent) : QObject(parent) {}

void QueryEngine::bindConnectionPool(SQLiteConnectionPool* connectionPool) { connectionPool_ = connectionPool; }

QVariantList QueryEngine::execute(const QString& databasePath, const QString& sql) {
    QVariantMap metadata;
    metadata.insert(QStringLiteral("database"), connectionPool_ ? connectionPool_->openConnection(databasePath) : databasePath);
    metadata.insert(QStringLiteral("sql"), sql);
    metadata.insert(QStringLiteral("status"), QStringLiteral("planned"));
    return {metadata};
}
}  // namespace webide
