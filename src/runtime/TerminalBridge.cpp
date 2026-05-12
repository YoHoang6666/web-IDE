#include "TerminalBridge.h"

#include "ProcessManager.h"

namespace webide {
TerminalBridge::TerminalBridge(ProcessManager* processManager, QObject* parent)
    : QObject(parent), processManager_(processManager) {}

QString TerminalBridge::runShellCommand(const QString& command, const QString& workingDirectory) {
    if (!processManager_ || command.trimmed().isEmpty()) {
        return {};
    }

#ifdef Q_OS_WIN
    return processManager_->startProcess(QStringLiteral("shell"),
                                         command,
                                         QStringLiteral("cmd.exe"),
                                         {QStringLiteral("/C"), command},
                                         workingDirectory);
#else
    return processManager_->startProcess(QStringLiteral("shell"),
                                         command,
                                         QStringLiteral("bash"),
                                         {QStringLiteral("-lc"), command},
                                         workingDirectory);
#endif
}
}  // namespace webide

