#include "PreviewSessionManager.h"

namespace webide {
PreviewSessionManager::PreviewSessionManager(QObject* parent) : QObject(parent) {}

void PreviewSessionManager::assignTarget(const QString& documentPath, const QUrl& url) { targets_[documentPath] = url; }

QUrl PreviewSessionManager::targetFor(const QString& documentPath) const { return targets_.value(documentPath); }
}  // namespace webide
