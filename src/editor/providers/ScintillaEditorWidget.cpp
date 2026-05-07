#include "ScintillaEditorWidget.h"

#include <QLabel>
#include <QVBoxLayout>

namespace webide {
ScintillaEditorWidget::ScintillaEditorWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Scintilla editor backend placeholder"), this));
}
}  // namespace webide
