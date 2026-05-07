#include "LiveReloadController.h"

#include "PreviewPane.h"

namespace webide {
LiveReloadController::LiveReloadController(QObject* parent) : QObject(parent) {}

void LiveReloadController::bindPreviewPane(PreviewPane* previewPane) { previewPane_ = previewPane; }

void LiveReloadController::handleChangeSet(const ChangeSet& changeSet) {
    if (!previewPane_) {
        return;
    }

    if (changeSet.kind == ChangeKind::Modified || changeSet.kind == ChangeKind::ReloadRequested) {
        previewPane_->loadTarget(changeSet.resourcePath);
    }
}
}  // namespace webide
