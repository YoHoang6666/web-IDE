#include "MonacoWebViewBridge.h"

#include <QLabel>
#include <QVBoxLayout>

namespace webide {
MonacoWebViewBridge::MonacoWebViewBridge(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Monaco embedded webview bridge placeholder"), this));
}
}  // namespace webide
