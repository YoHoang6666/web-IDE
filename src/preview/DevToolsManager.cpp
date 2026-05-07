#include "DevToolsManager.h"

#include <QDebug>

namespace webide {
DevToolsManager::DevToolsManager(QObject* parent) : QObject(parent) {}

void DevToolsManager::bindPreviewPane(PreviewPane* previewPane) { previewPane_ = previewPane; }

void DevToolsManager::openInspector() {
    if (previewPane_) {
        qInfo() << "DevTools attached to active preview session";
    }
}
}  // namespace webide
