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

void DockLayoutManager::applyEditorPreviewLayout() {
    restoreDefaultLayout();
    window_->setDockNestingEnabled(true);
    window_->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    window_->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}

void DockLayoutManager::applyEditorDatabaseLayout() {
    restoreDefaultLayout();
    window_->setDockNestingEnabled(false);
    window_->setCorner(Qt::TopRightCorner, Qt::BottomDockWidgetArea);
    window_->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}
}  // namespace webide
