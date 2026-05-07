#include "WorkspaceManager.h"

namespace webide {
WorkspaceManager::WorkspaceManager(QObject* parent) : QObject(parent) {}

void WorkspaceManager::openWorkspace(const QString& path) {
    currentWorkspace_ = path;
    if (!recentWorkspaces_.contains(path)) {
        recentWorkspaces_.prepend(path);
    }
    emit workspaceOpened(path);
}

QString WorkspaceManager::currentWorkspace() const { return currentWorkspace_; }

QStringList WorkspaceManager::recentWorkspaces() const { return recentWorkspaces_; }
}  // namespace webide
