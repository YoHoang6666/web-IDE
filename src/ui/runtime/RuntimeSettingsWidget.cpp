#include "RuntimeSettingsWidget.h"

#include <QFormLayout>
#include <QLineEdit>

namespace webide {
RuntimeSettingsWidget::RuntimeSettingsWidget(QWidget* parent)
    : QWidget(parent),
      phpPath_(new QLineEdit(this)),
      nodePath_(new QLineEdit(this)),
      pythonPath_(new QLineEdit(this)) {

    auto* layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addRow(tr("PHP executable"), phpPath_);
    layout->addRow(tr("Node executable"), nodePath_);
    layout->addRow(tr("Python executable"), pythonPath_);
}

void RuntimeSettingsWidget::setPhpPath(const QString& path) {
    phpPath_->setText(path);
}

void RuntimeSettingsWidget::setNodePath(const QString& path) {
    nodePath_->setText(path);
}

void RuntimeSettingsWidget::setPythonPath(const QString& path) {
    pythonPath_->setText(path);
}
}  // namespace webide
