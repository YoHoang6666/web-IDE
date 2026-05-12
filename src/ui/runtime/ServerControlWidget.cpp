#include "ServerControlWidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

namespace webide {
ServerControlWidget::ServerControlWidget(QWidget* parent)
    : QWidget(parent),
      runtimeBox_(new QComboBox(this)),
      commandEdit_(new QLineEdit(this)),
      portSpin_(new QSpinBox(this)),
      startButton_(new QPushButton(tr("Start"), this)),
      stopButton_(new QPushButton(tr("Stop"), this)),
      restartButton_(new QPushButton(tr("Restart"), this)),
      killButton_(new QPushButton(tr("Kill"), this)),
      openBrowserButton_(new QPushButton(tr("Open Browser"), this)),
      openTerminalButton_(new QPushButton(tr("Open Terminal"), this)) {
    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(6);
    layout->setVerticalSpacing(6);

    runtimeBox_->addItem(tr("PHP"), QStringLiteral("php"));
    runtimeBox_->addItem(tr("Node.js"), QStringLiteral("node"));
    runtimeBox_->addItem(tr("Python"), QStringLiteral("python"));
    runtimeBox_->addItem(tr("Static"), QStringLiteral("static"));

    commandEdit_->setPlaceholderText(tr("Command / script / module"));
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(3000);

    layout->addWidget(runtimeBox_, 0, 0, 1, 2);
    layout->addWidget(commandEdit_, 1, 0);
    layout->addWidget(portSpin_, 1, 1);

    layout->addWidget(startButton_, 2, 0);
    layout->addWidget(stopButton_, 2, 1);
    layout->addWidget(restartButton_, 3, 0);
    layout->addWidget(killButton_, 3, 1);
    layout->addWidget(openBrowserButton_, 4, 0);
    layout->addWidget(openTerminalButton_, 4, 1);

    connect(startButton_, &QPushButton::clicked, this, [this]() {
        emit startRequested(runtimeBox_->currentData().toString(), commandEdit_->text().trimmed(), portSpin_->value());
    });
    connect(stopButton_, &QPushButton::clicked, this, &ServerControlWidget::stopRequested);
    connect(restartButton_, &QPushButton::clicked, this, &ServerControlWidget::restartRequested);
    connect(killButton_, &QPushButton::clicked, this, &ServerControlWidget::killRequested);
    connect(openBrowserButton_, &QPushButton::clicked, this, [this]() { emit openBrowserRequested(portSpin_->value()); });
    connect(openTerminalButton_, &QPushButton::clicked, this, [this]() { emit openTerminalRequested(commandEdit_->text().trimmed()); });
}
}  // namespace webide

