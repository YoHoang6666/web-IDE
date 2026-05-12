#pragma once

#include <QObject>

namespace webide {
class ProcessManager;
class RuntimeManager;

class LocalServerManager : public QObject {
    Q_OBJECT

public:
    explicit LocalServerManager(RuntimeManager* runtimeManager, ProcessManager* processManager, QObject* parent = nullptr);

    QString startPhpServer(const QString& workspacePath, int port);
    QString startNodeScript(const QString& workspacePath, const QString& scriptName, int portHint = 0);
    QString startPythonModule(const QString& workspacePath, const QString& module, int portHint = 0);
    QString startStaticServer(const QString& workspacePath, int port);

private:
    QString runtimeExecutable(const QString& runtimeId) const;

    RuntimeManager* runtimeManager_;
    ProcessManager* processManager_;
};
}  // namespace webide

