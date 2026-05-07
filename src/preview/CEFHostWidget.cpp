#include "CEFHostWidget.h"

#include <QLabel>
#include <QVBoxLayout>

namespace webide {
CEFHostWidget::CEFHostWidget(QWidget* parent) : QWidget(parent), stateLabel_(new QLabel(tr("CEF preview host idle"), this)) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(stateLabel_);
}

void CEFHostWidget::navigate(const QString& target) { stateLabel_->setText(tr("Previewing: %1").arg(target)); }
}  // namespace webide
