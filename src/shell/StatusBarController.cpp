#include "StatusBarController.h"

#include <QLabel>
#include <QStatusBar>

namespace webide {
StatusBarController::StatusBarController(QStatusBar* statusBar, QObject* parent)
    : QObject(parent), statusBar_(statusBar), modeLabel_(new QLabel(tr("Ready"), statusBar)) {
    statusBar_->addPermanentWidget(modeLabel_);
}

void StatusBarController::showWorkspaceMessage(const QString& message) { statusBar_->showMessage(message, 4000); }

void StatusBarController::setMode(const QString& mode) { modeLabel_->setText(mode); }
}  // namespace webide
