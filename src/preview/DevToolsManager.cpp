#include "DevToolsManager.h"

#include "core/Logger.h"

namespace webide {
DevToolsManager::DevToolsManager(QObject* parent) : QObject(parent) {}

void DevToolsManager::bindPreviewPane(PreviewPane* previewPane) { previewPane_ = previewPane; }

void DevToolsManager::openInspector() {
    if (previewPane_) {
        Logger::global().info(QStringLiteral("DevTools attached to active preview session"), QStringLiteral("preview"));
    }
}
}  // namespace webide
