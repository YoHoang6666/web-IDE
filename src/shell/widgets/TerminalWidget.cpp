#include "TerminalWidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "runtime/AnsiTextFormatter.h"
#include "runtime/TerminalSession.h"

namespace webide {
TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent),
      output_(new QTextEdit(this)),
      input_(new QLineEdit(this)),
      stopButton_(new QPushButton(tr("Stop"), this)),
      session_(new TerminalSession(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    output_->setReadOnly(true);
    output_->setAcceptRichText(true);
    QFont font;
    font.setStyleHint(QFont::Monospace);
    font.setFamilies({QStringLiteral("Consolas"), QStringLiteral("Monospace"), QStringLiteral("JetBrains Mono")});
    font.setPointSize(10);
    output_->setFont(font);
    input_->setPlaceholderText(tr("Enter command (npm, node, python, git, ... )"));
    stopButton_->setEnabled(false);

    auto* inputRow = new QHBoxLayout();
    inputRow->addWidget(input_, 1);
    inputRow->addWidget(stopButton_);

    layout->addWidget(output_, 1);
    layout->addLayout(inputRow);

    connect(input_, &QLineEdit::returnPressed, this, [this]() {
        const QString command = input_->text().trimmed();
        if (command.isEmpty()) {
            return;
        }
        executeCommand(command);
        input_->clear();
    });

    connect(stopButton_, &QPushButton::clicked, this, &TerminalWidget::cancelActive);

    connect(session_, &TerminalSession::outputReady, this, &TerminalWidget::appendOutput);
    connect(session_, &TerminalSession::statusMessage, this, &TerminalWidget::statusMessage);
    connect(session_, &TerminalSession::runningChanged, this, &TerminalWidget::updateStopState);
    connect(session_, &TerminalSession::processExited, this, [this](const ProcessExitState& state) {
        appendOutput(tr("[process %1, code %2]\\n").arg(state.message).arg(state.exitCode), state.reason != ProcessExitReason::Exited);
    });

    const auto shell = session_->shell();
    if (!shell.program.isEmpty()) {
        emit statusMessage(tr("Shell: %1").arg(shell.name));
    }
}

void TerminalWidget::executeCommand(const QString& command, const TerminalCommandOptions& options) {
    if (command.trimmed().isEmpty()) {
        return;
    }
    lastCommand_ = command;
    appendOutput(tr("$ %1\\n").arg(command), false);
    if (!session_->runCommand(command, options)) {
        appendOutput(tr("Command rejected (already running).\\n"), true);
    }
}

void TerminalWidget::cancelActive() { session_->cancelActive(); }

void TerminalWidget::setWorkingDirectory(const QString& path) { session_->setWorkingDirectory(path); }

void TerminalWidget::setEnvironment(const QProcessEnvironment& environment) { session_->setEnvironment(environment); }

void TerminalWidget::appendOutput(const QString& text, bool isError) {
    formatter_.append(output_, text, isError);
}

void TerminalWidget::updateStopState(bool running) { stopButton_->setEnabled(running); }
}  // namespace webide
