#pragma once

#include <QObject>
#include <QStringList>

namespace webide {
class SQLiteConnectionPool;

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject* parent = nullptr);

    void bindConnectionPool(SQLiteConnectionPool* connectionPool);
    void attachDatabase(const QString& path);
    QStringList attachedDatabases() const;

signals:
    void databaseAttached(const QString& path);

private:
    SQLiteConnectionPool* connectionPool_ = nullptr;
    QStringList attachedDatabases_;
};
}  // namespace webide
