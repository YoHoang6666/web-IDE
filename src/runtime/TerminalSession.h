#pragma once

#include <QObject>
#include <QProcessEnvironment>
#include <QUuid>

#include "ProcessManager.h"
#include "RuntimeSession.h"
#include "ShellDetector.h"

namespace webide {
struct TerminalCommandOptions {
    bool detached = false;
    RestartPolicy restartPolicy;
};

class TerminalSession : public QObject {
    Q_OBJECT

public:
    explicit TerminalSession(QObject* parent = nullptr);

    bool runCommand(const QString& command, const TerminalCommandOptions& options = {});
    void cancelActive();
    bool isRunning() const;

    void setWorkingDirectory(const QString& path);
    void setEnvironment(const QProcessEnvironment& environment);

    ShellDescriptor shell() const;
    RuntimeSession* runtimeSession() const;

signals:
    void outputReady(const QString& text, bool isError);
    void statusMessage(const QString& message);
    void processExited(const ProcessExitState& state);
    void runningChanged(bool running);
    void processRestarted(int attempt);

private:
    ProcessManager* processManager_;
    RuntimeSession* runtimeSession_;
    ShellDescriptor shell_;
    QUuid activeProcessId_;
    qint64 activePid_ = 0;
    bool running_ = false;
};
}  // namespace webide
