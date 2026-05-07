#include "EditorHost.h"

#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

namespace webide {
EditorHost::EditorHost(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    splitter_ = new QSplitter(Qt::Horizontal, this);
    primaryEditor_ = new QLabel(tr("Primary code editor surface"), splitter_);
    secondaryEditor_ = new QLabel(tr("Secondary split editor surface"), splitter_);
    splitter_->addWidget(primaryEditor_);
    splitter_->addWidget(secondaryEditor_);
    layout->addWidget(splitter_);
}

void EditorHost::enableSecondarySplit(bool enabled) {
    secondaryEditor_->setVisible(enabled);
}
}  // namespace webide
