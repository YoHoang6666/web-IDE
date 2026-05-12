#pragma once

#include <QObject>

namespace webide {
class ProcessManager;

class TerminalBridge : public QObject {
    Q_OBJECT

public:
    explicit TerminalBridge(ProcessManager* processManager, QObject* parent = nullptr);

    QString runShellCommand(const QString& command, const QString& workingDirectory = {});

private:
    ProcessManager* processManager_;
};
}  // namespace webide

