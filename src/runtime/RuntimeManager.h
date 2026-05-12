#pragma once

#include <QObject>
#include <QHash>

#include "RuntimeEnvironment.h"

namespace webide {
class RuntimeBase;
class ProcessManager;
class LocalServerManager;
class TerminalBridge;

class RuntimeManager : public QObject {
    Q_OBJECT

public:
    explicit RuntimeManager(QObject* parent = nullptr);
    ~RuntimeManager() override;

    void detectRuntimes();
    QList<RuntimeEnvironment> environments() const;
    RuntimeEnvironment environment(const QString& runtimeId) const;

    ProcessManager* processManager() const;
    LocalServerManager* localServerManager() const;
    TerminalBridge* terminalBridge() const;

signals:
    void runtimesChanged(const QList<webide::RuntimeEnvironment>& environments);
    void runtimeEnvironmentChanged(const webide::RuntimeEnvironment& environment);

private:
    void registerRuntime(RuntimeBase* runtime);

    QList<RuntimeBase*> runtimes_;
    QHash<QString, RuntimeEnvironment> environments_;
    ProcessManager* processManager_;
    LocalServerManager* localServerManager_;
    TerminalBridge* terminalBridge_;
};
}  // namespace webide

