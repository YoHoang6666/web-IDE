#pragma once

#include <QWidget>

class QLineEdit;
class QPlainTextEdit;
class QProcess;

namespace webide {
class TerminalWidget : public QWidget {
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);

    void executeCommand(const QString& command);

signals:
    void statusMessage(const QString& message);

private:
    QPlainTextEdit* output_;
    QLineEdit* input_;
    QProcess* process_;
};
}  // namespace webide
