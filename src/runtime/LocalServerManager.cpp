#include "LocalServerManager.h"

#include "ProcessManager.h"
#include "RuntimeManager.h"

namespace webide {
LocalServerManager::LocalServerManager(RuntimeManager* runtimeManager, ProcessManager* processManager, QObject* parent)
    : QObject(parent), runtimeManager_(runtimeManager), processManager_(processManager) {}

QString LocalServerManager::startPhpServer(const QString& workspacePath, int port) {
    const QString php = runtimeExecutable(QStringLiteral("php"));
    if (php.isEmpty() || !processManager_) {
        return {};
    }
    return processManager_->startProcess(QStringLiteral("php"),
                                         QStringLiteral("php -S localhost:%1").arg(port),
                                         php,
                                         {QStringLiteral("-S"), QStringLiteral("localhost:%1").arg(port)},
                                         workspacePath,
                                         port);
}

QString LocalServerManager::startNodeScript(const QString& workspacePath, const QString& scriptName, int portHint) {
    const QString node = runtimeExecutable(QStringLiteral("node"));
    if (node.isEmpty() || !processManager_ || scriptName.isEmpty()) {
        return {};
    }
    return processManager_->startProcess(QStringLiteral("node"),
                                         QStringLiteral("node %1").arg(scriptName),
                                         node,
                                         {scriptName},
                                         workspacePath,
                                         portHint);
}

QString LocalServerManager::startPythonModule(const QString& workspacePath, const QString& module, int portHint) {
    const QString python = runtimeExecutable(QStringLiteral("python"));
    if (python.isEmpty() || !processManager_ || module.isEmpty()) {
        return {};
    }
    return processManager_->startProcess(QStringLiteral("python"),
                                         QStringLiteral("python -m %1").arg(module),
                                         python,
                                         {QStringLiteral("-m"), module},
                                         workspacePath,
                                         portHint);
}

QString LocalServerManager::startStaticServer(const QString& workspacePath, int port) {
    const QString python = runtimeExecutable(QStringLiteral("python"));
    if (!python.isEmpty() && processManager_) {
        return processManager_->startProcess(QStringLiteral("python"),
                                             QStringLiteral("python -m http.server %1").arg(port),
                                             python,
                                             {QStringLiteral("-m"), QStringLiteral("http.server"), QString::number(port)},
                                             workspacePath,
                                             port);
    }
    return {};
}

QString LocalServerManager::runtimeExecutable(const QString& runtimeId) const {
    if (!runtimeManager_) {
        return {};
    }
    const RuntimeEnvironment environment = runtimeManager_->environment(runtimeId);
    return environment.executablePath;
}
}  // namespace webide

