#include "PreviewPane.h"

#include <QVBoxLayout>

#include "CEFHostWidget.h"

namespace webide {
PreviewPane::PreviewPane(QWidget* parent) : QWidget(parent), browserHost_(new CEFHostWidget(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(browserHost_);
}

void PreviewPane::loadTarget(const QString& target) {
    browserHost_->navigate(target);
    emit previewTargetChanged(target);
}

CEFHostWidget* PreviewPane::browserHost() const { return browserHost_; }
}  // namespace webide
