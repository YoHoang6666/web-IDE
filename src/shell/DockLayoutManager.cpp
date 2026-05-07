#include "DockLayoutManager.h"

#include <QMainWindow>

namespace webide {
DockLayoutManager::DockLayoutManager(QMainWindow* window) : QObject(window), window_(window) {}

void DockLayoutManager::captureDefaultState(const QByteArray& state) { defaultState_ = state; }

void DockLayoutManager::restoreDefaultLayout() {
    if (!defaultState_.isEmpty()) {
        window_->restoreState(defaultState_);
    }
}

void DockLayoutManager::applyEditorPreviewLayout() { restoreDefaultLayout(); }

void DockLayoutManager::applyEditorDatabaseLayout() { restoreDefaultLayout(); }
}  // namespace webide
