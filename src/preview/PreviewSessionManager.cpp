#include "PreviewSessionManager.h"

namespace webide {
PreviewSessionManager::PreviewSessionManager(QObject* parent) : QObject(parent) {}

void PreviewSessionManager::assignTarget(const QString& documentPath, const QUrl& url) {
    PreviewSession session = sessions_.value(documentPath);
    session.documentPath = documentPath;
    session.targetUrl = url;
    session.status = PreviewServerStatus::Running;
    sessions_.insert(documentPath, session);
}

void PreviewSessionManager::assignSession(const PreviewSession& session) {
    if (session.documentPath.isEmpty()) {
        return;
    }
    sessions_.insert(session.documentPath, session);
}

PreviewSession PreviewSessionManager::sessionFor(const QString& documentPath) const {
    return sessions_.value(documentPath);
}

void PreviewSessionManager::updateStatus(const QString& documentPath, PreviewServerStatus status, const QString& detail) {
    if (!sessions_.contains(documentPath)) {
        return;
    }
    PreviewSession session = sessions_.value(documentPath);
    session.status = status;
    session.statusDetail = detail;
    sessions_.insert(documentPath, session);
}
}  // namespace webide
