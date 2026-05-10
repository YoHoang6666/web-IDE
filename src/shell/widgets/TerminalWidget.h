#pragma once

#include <QWidget>

#include "runtime/AnsiTextFormatter.h"
#include "runtime/TerminalSession.h"

class QLineEdit;
class QPushButton;
class QTextEdit;
class QProcessEnvironment;

namespace webide {

class TerminalWidget : public QWidget {
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);

    void executeCommand(const QString& command, const TerminalCommandOptions& options = {});
    void cancelActive();
    void setWorkingDirectory(const QString& path);
    void setEnvironment(const QProcessEnvironment& environment);

signals:
    void statusMessage(const QString& message);

private:
    void appendOutput(const QString& text, bool isError);
    void updateStopState(bool running);

    QTextEdit* output_;
    QLineEdit* input_;
    QPushButton* stopButton_;
    TerminalSession* session_;
    AnsiTextFormatter formatter_;
    QString lastCommand_;
};
}  // namespace webide
