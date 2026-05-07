#pragma once

#include <QObject>
#include <QString>

#include "sync/ChangeSet.h"

class QTimer;

namespace webide {
class FileWatcherService : public QObject {
    Q_OBJECT

public:
    explicit FileWatcherService(QObject* parent = nullptr);

    void enqueueExternalChange(const QString& path);

signals:
    void changeDetected(const ChangeSet& changeSet);

private:
    QString pendingPath_;
    QTimer* debounceTimer_;
};
}  // namespace webide
