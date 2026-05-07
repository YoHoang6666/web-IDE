#include "DocumentManager.h"

#include "src/workspace/FileSystemService.h"

namespace webide {
DocumentManager::DocumentManager(QObject* parent) : QObject(parent) {}

void DocumentManager::bindFileSystem(FileSystemService* fileSystemService) { fileSystemService_ = fileSystemService; }

void DocumentManager::updateBuffer(const QString& path, const QString& content) { buffers_[path] = content; }

bool DocumentManager::saveDocument(const QString& path) {
    if (!fileSystemService_ || !buffers_.contains(path)) {
        return false;
    }

    if (fileSystemService_->writeFile(path, buffers_.value(path))) {
        emit documentSaved(path, buffers_.value(path));
        return true;
    }
    return false;
}

QString DocumentManager::contentFor(const QString& path) const { return buffers_.value(path); }
}  // namespace webide
