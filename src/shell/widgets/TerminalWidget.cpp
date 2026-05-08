#include "TerminalWidget.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QVBoxLayout>

namespace webide {
TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent), output_(new QPlainTextEdit(this)), input_(new QLineEdit(this)), process_(new QProcess(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    output_->setReadOnly(true);
    input_->setPlaceholderText(tr("Enter command (npm, node, python, git, ... )"));

    layout->addWidget(output_, 1);
    layout->addWidget(input_);

    connect(input_, &QLineEdit::returnPressed, this, [this]() {
        const QString command = input_->text().trimmed();
        if (command.isEmpty()) {
            return;
        }
        executeCommand(command);
        input_->clear();
    });

    connect(process_, &QProcess::readyReadStandardOutput, this, [this]() {
        output_->appendPlainText(QString::fromLocal8Bit(process_->readAllStandardOutput()));
    });

    connect(process_, &QProcess::readyReadStandardError, this, [this]() {
        output_->appendPlainText(QString::fromLocal8Bit(process_->readAllStandardError()));
    });

    connect(process_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus) {
        output_->appendPlainText(tr("[process exited with code %1]\n").arg(code));
    });
}

void TerminalWidget::executeCommand(const QString& command) {
    if (process_->state() != QProcess::NotRunning) {
        output_->appendPlainText(tr("A command is already running.\n"));
        return;
    }

    output_->appendPlainText(tr("$ %1").arg(command));

#ifdef Q_OS_WIN
    process_->start(QStringLiteral("cmd.exe"), {QStringLiteral("/C"), command});
#else
    process_->start(QStringLiteral("bash"), {QStringLiteral("-lc"), command});
#endif

    emit statusMessage(tr("Running: %1").arg(command));
}
}  // namespace webide
