#pragma once

#include <QObject>
#include <QString>

namespace webide {
class SQLiteConnectionPool : public QObject {
    Q_OBJECT

public:
    explicit SQLiteConnectionPool(QObject* parent = nullptr);

    QString openConnection(const QString& databasePath);
};
}  // namespace webide
