#include "SyncCoordinator.h"

#include "src/database/DatabaseWatcherService.h"
#include "src/preview/LiveReloadController.h"
#include "src/preview/PreviewPane.h"
#include "src/workspace/FileWatcherService.h"

namespace webide {
SyncCoordinator::SyncCoordinator(QObject* parent) : QObject(parent) {}

void SyncCoordinator::attachFileWatcher(FileWatcherService* watcher) {
    connect(watcher, &FileWatcherService::changeDetected, this, &SyncCoordinator::processChange);
}

void SyncCoordinator::attachDatabaseWatcher(DatabaseWatcherService* watcher) {
    connect(watcher, &DatabaseWatcherService::changeDetected, this, &SyncCoordinator::processChange);
}

void SyncCoordinator::attachLiveReloadController(LiveReloadController* controller) {
    liveReloadController_ = controller;
    connect(this, &SyncCoordinator::previewReloadRequested, controller, &LiveReloadController::handleChangeSet);
}

void SyncCoordinator::attachPreview(PreviewPane* previewPane) { previewPane_ = previewPane; }

void SyncCoordinator::processChange(const ChangeSet& changeSet) {
    Q_UNUSED(previewPane_);
    emit previewReloadRequested(changeSet);
}
}  // namespace webide
