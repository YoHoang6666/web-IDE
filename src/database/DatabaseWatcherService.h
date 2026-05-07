#pragma once

#include <QObject>

#include "sync/ChangeSet.h"

namespace webide {
class DatabaseWatcherService : public QObject {
    Q_OBJECT

public:
    explicit DatabaseWatcherService(QObject* parent = nullptr);
    void notifyDatabaseTouched(const QString& databasePath);

signals:
    void changeDetected(const ChangeSet& changeSet);
};
}  // namespace webide
