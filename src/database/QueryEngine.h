#pragma once

#include <QObject>
#include <QVariantList>

namespace webide {
class SQLiteConnectionPool;

class QueryEngine : public QObject {
    Q_OBJECT

public:
    explicit QueryEngine(QObject* parent = nullptr);

    void bindConnectionPool(SQLiteConnectionPool* connectionPool);
    QVariantList execute(const QString& databasePath, const QString& sql);

private:
    SQLiteConnectionPool* connectionPool_ = nullptr;
};
}  // namespace webide
