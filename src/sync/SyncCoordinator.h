#pragma once

#include <QObject>

#include "ChangeSet.h"

namespace webide {
class FileWatcherService;
class DatabaseWatcherService;
class LiveReloadController;
class PreviewPane;

class SyncCoordinator : public QObject {
    Q_OBJECT

public:
    explicit SyncCoordinator(QObject* parent = nullptr);

    void attachFileWatcher(FileWatcherService* watcher);
    void attachDatabaseWatcher(DatabaseWatcherService* watcher);
    void attachLiveReloadController(LiveReloadController* controller);
    void attachPreview(PreviewPane* previewPane);

public slots:
    void processChange(const ChangeSet& changeSet);

signals:
    void previewReloadRequested(const ChangeSet& changeSet);

private:
    LiveReloadController* liveReloadController_ = nullptr;
    PreviewPane* previewPane_ = nullptr;
};
}  // namespace webide
