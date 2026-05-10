#pragma once

#include <QHash>
#include <QObject>
#include <QProcessEnvironment>
#include <QUuid>

class QProcess;

namespace webide {

enum class ProcessExitReason { Exited, Crashed, FailedToStart, Cancelled, Detached };

struct ProcessExitState {
    ProcessExitReason reason = ProcessExitReason::Exited;
    int exitCode = 0;
    QString message;
};

enum class RestartMode { Never, OnFailure, Always };

struct RestartPolicy {
    RestartMode mode = RestartMode::Never;
    int maxRetries = 0;
    int delayMs = 2000;
};

struct ProcessSpec {
    QString program;
    QStringList arguments;
    QString workingDirectory;
    QProcessEnvironment environment;
    bool detached = false;
    RestartPolicy restartPolicy;
    QString displayName;
};

class ProcessManager : public QObject {
    Q_OBJECT

public:
    explicit ProcessManager(QObject* parent = nullptr);

    QUuid start(const ProcessSpec& spec);
    bool cancel(const QUuid& id);
    bool restart(const QUuid& id);
    bool isRunning(const QUuid& id) const;

signals:
    void processStarted(const QUuid& id, qint64 pid);
    void processOutput(const QUuid& id, const QString& text, bool isStdErr);
    void processExited(const QUuid& id, const ProcessExitState& state);
    void processRestarted(const QUuid& id, int attempt);

private:
    struct ProcessEntry {
        ProcessSpec spec;
        QProcess* process = nullptr;
        int restartCount = 0;
        bool cancelRequested = false;
        bool restartRequested = false;
        qint64 pid = 0;
    };

    void startProcess(const QUuid& id);
    void handleExit(const QUuid& id, int exitCode, QProcess::ExitStatus status);
    void scheduleRestart(const QUuid& id);
    bool shouldRestart(const ProcessEntry& entry, const ProcessExitState& state) const;

    QHash<QUuid, ProcessEntry> processes_;
};
}  // namespace webide
