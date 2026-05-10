#include "ShellDetector.h"

#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include "core/Logger.h"

namespace webide {
ShellDescriptor ShellDetector::detectDefault() {
    ShellDescriptor descriptor;

#ifdef Q_OS_WIN
    const QString pwsh = QStandardPaths::findExecutable(QStringLiteral("pwsh"));
    const QString powershell = QStandardPaths::findExecutable(QStringLiteral("powershell"));
    const QString cmd = QStandardPaths::findExecutable(QStringLiteral("cmd"));

    if (!pwsh.isEmpty()) {
        descriptor.name = QStringLiteral("PowerShell (pwsh)");
        descriptor.program = pwsh;
        descriptor.arguments = {QStringLiteral("-NoProfile"), QStringLiteral("-Command")};
    } else if (!powershell.isEmpty()) {
        descriptor.name = QStringLiteral("Windows PowerShell");
        descriptor.program = powershell;
        descriptor.arguments = {QStringLiteral("-NoProfile"), QStringLiteral("-Command")};
    } else if (!cmd.isEmpty()) {
        descriptor.name = QStringLiteral("Command Prompt");
        descriptor.program = cmd;
        descriptor.arguments = {QStringLiteral("/C")};
        descriptor.supportsAnsi = false;
    } else {
        descriptor.name = QStringLiteral("Command Prompt");
        descriptor.program = QStringLiteral("cmd.exe");
        descriptor.arguments = {QStringLiteral("/C")};
        descriptor.supportsAnsi = false;
        Logger::global().warning(QStringLiteral("Shell auto-detect failed; falling back to cmd.exe"), QStringLiteral("terminal"));
    }
#else
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString envShell = env.value(QStringLiteral("SHELL"));
    if (!envShell.isEmpty() && QFileInfo::exists(envShell)) {
        descriptor.name = QFileInfo(envShell).fileName();
        descriptor.program = envShell;
    } else {
        const QString fallback = QStandardPaths::findExecutable(QStringLiteral("bash"));
        const QString zsh = QStandardPaths::findExecutable(QStringLiteral("zsh"));
        const QString sh = QStandardPaths::findExecutable(QStringLiteral("sh"));
        descriptor.program = !fallback.isEmpty() ? fallback : (!zsh.isEmpty() ? zsh : sh);
        descriptor.name = QFileInfo(descriptor.program).fileName();
        if (descriptor.program.isEmpty()) {
            descriptor.program = QStringLiteral("/bin/sh");
            descriptor.name = QStringLiteral("sh");
            Logger::global().warning(QStringLiteral("Shell auto-detect failed; falling back to /bin/sh"),
                                     QStringLiteral("terminal"));
        }
    }
    descriptor.arguments = {QStringLiteral("-lc")};
#endif

    return descriptor;
}
}  // namespace webide
