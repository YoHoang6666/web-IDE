#include "EditorTabManager.h"

namespace webide {
EditorTabManager::EditorTabManager(QObject* parent) : QObject(parent) {}

void EditorTabManager::openTab(const QString& path) {
    if (!openTabs_.contains(path)) {
        openTabs_.append(path);
        emit tabsChanged();
    }
}

void EditorTabManager::closeTab(const QString& path) {
    if (openTabs_.removeAll(path) > 0) {
        emit tabsChanged();
    }
}

QStringList EditorTabManager::openTabs() const { return openTabs_; }
}  // namespace webide
