#include "ProcessManager.h"

#include <QProcess>
#include <QTimer>

#include "core/Logger.h"

namespace webide {
namespace {
QString reasonLabel(ProcessExitReason reason) {
    switch (reason) {
        case ProcessExitReason::Exited:
            return QStringLiteral("exited");
        case ProcessExitReason::Crashed:
            return QStringLiteral("crashed");
        case ProcessExitReason::FailedToStart:
            return QStringLiteral("failed-to-start");
        case ProcessExitReason::Cancelled:
            return QStringLiteral("cancelled");
        case ProcessExitReason::Detached:
            return QStringLiteral("detached");
    }
    return QStringLiteral("unknown");
}
}  // namespace

ProcessManager::ProcessManager(QObject* parent) : QObject(parent) {}

QUuid ProcessManager::start(const ProcessSpec& spec) {
    const QUuid id = QUuid::createUuid();

    if (spec.program.isEmpty()) {
        ProcessExitState state{ProcessExitReason::FailedToStart, -1, QStringLiteral("No program specified")};
        emit processExited(id, state);
        Logger::global().error(QStringLiteral("Process failed to start: missing program"), QStringLiteral("runtime"));
        return id;
    }

    if (spec.detached) {
        qint64 pid = 0;
        const bool started = QProcess::startDetached(spec.program, spec.arguments, spec.workingDirectory, &pid);
        if (!started) {
            ProcessExitState state{ProcessExitReason::FailedToStart, -1, QStringLiteral("Detached start failed")};
            emit processExited(id, state);
            Logger::global().error(QStringLiteral("Detached process failed to start: %1").arg(spec.program),
                                   QStringLiteral("runtime"));
            return id;
        }
        emit processStarted(id, pid);
        ProcessExitState state{ProcessExitReason::Detached, 0, QStringLiteral("Process running in background")};
        emit processExited(id, state);
        Logger::global().info(QStringLiteral("Detached process started (%1) pid=%2").arg(spec.program).arg(pid),
                              QStringLiteral("runtime"));
        return id;
    }

    ProcessEntry entry;
    entry.spec = spec;
    entry.process = new QProcess(this);
    processes_.insert(id, entry);
    startProcess(id);
    return id;
}

bool ProcessManager::cancel(const QUuid& id) {
    auto it = processes_.find(id);
    if (it == processes_.end() || !it->process) {
        return false;
    }
    it->cancelRequested = true;
    it->restartRequested = false;
    it->process->terminate();
    QTimer::singleShot(3000, it->process, [process = it->process]() {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
    });
    return true;
}

bool ProcessManager::restart(const QUuid& id) {
    auto it = processes_.find(id);
    if (it == processes_.end()) {
        return false;
    }
    it->restartRequested = true;
    if (it->process && it->process->state() != QProcess::NotRunning) {
        it->process->terminate();
        QTimer::singleShot(2000, it->process, [process = it->process]() {
            if (process->state() != QProcess::NotRunning) {
                process->kill();
            }
        });
        return true;
    }
    startProcess(id);
    return true;
}

bool ProcessManager::isRunning(const QUuid& id) const {
    const auto it = processes_.find(id);
    if (it == processes_.end() || !it->process) {
        return false;
    }
    return it->process->state() != QProcess::NotRunning;
}

void ProcessManager::startProcess(const QUuid& id) {
    auto it = processes_.find(id);
    if (it == processes_.end()) {
        return;
    }

    ProcessEntry& entry = it.value();
    entry.cancelRequested = false;
    entry.process->setProgram(entry.spec.program);
    entry.process->setArguments(entry.spec.arguments);
    if (!entry.spec.workingDirectory.isEmpty()) {
        entry.process->setWorkingDirectory(entry.spec.workingDirectory);
    }
    if (!entry.spec.environment.isEmpty()) {
        entry.process->setProcessEnvironment(entry.spec.environment);
    }

    connect(entry.process, &QProcess::started, this, [this, id]() {
        auto it = processes_.find(id);
        if (it == processes_.end() || !it->process) {
            return;
        }
        it->pid = it->process->processId();
        emit processStarted(id, it->pid);
    });

    connect(entry.process, &QProcess::readyReadStandardOutput, this, [this, id]() {
        auto it = processes_.find(id);
        if (it == processes_.end() || !it->process) {
            return;
        }
        const QString output = QString::fromUtf8(it->process->readAllStandardOutput());
        if (!output.isEmpty()) {
            emit processOutput(id, output, false);
        }
    });

    connect(entry.process, &QProcess::readyReadStandardError, this, [this, id]() {
        auto it = processes_.find(id);
        if (it == processes_.end() || !it->process) {
            return;
        }
        const QString output = QString::fromUtf8(it->process->readAllStandardError());
        if (!output.isEmpty()) {
            emit processOutput(id, output, true);
        }
    });

    connect(entry.process, &QProcess::errorOccurred, this, [this, id](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) {
            Logger::global().warning(QStringLiteral("Process error: %1").arg(static_cast<int>(error)),
                                     QStringLiteral("runtime"));
            return;
        }
        ProcessExitState state{ProcessExitReason::FailedToStart, -1, QStringLiteral("Failed to start")};
        emit processExited(id, state);
        Logger::global().error(QStringLiteral("Process failed to start"), QStringLiteral("runtime"));
        auto it = processes_.find(id);
        if (it != processes_.end()) {
            if (it->process) {
                it->process->deleteLater();
            }
            processes_.erase(it);
        }
    });

    connect(entry.process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, id](int exitCode, QProcess::ExitStatus status) { handleExit(id, exitCode, status); });

    entry.process->start();
}

void ProcessManager::handleExit(const QUuid& id, int exitCode, QProcess::ExitStatus status) {
    auto it = processes_.find(id);
    if (it == processes_.end()) {
        return;
    }

    ProcessEntry entry = it.value();
    ProcessExitReason reason = ProcessExitReason::Exited;
    if (entry.cancelRequested) {
        reason = ProcessExitReason::Cancelled;
    } else if (status == QProcess::CrashExit) {
        reason = ProcessExitReason::Crashed;
    }

    ProcessExitState state{reason, exitCode, QStringLiteral("Process %1 with code %2")
                                              .arg(reasonLabel(reason))
                                              .arg(exitCode)};
    emit processExited(id, state);

    if (reason == ProcessExitReason::Crashed || (reason == ProcessExitReason::Exited && exitCode != 0)) {
        Logger::global().error(QStringLiteral("Process exited (%1) code=%2").arg(reasonLabel(reason)).arg(exitCode),
                               QStringLiteral("runtime"));
    } else if (reason == ProcessExitReason::Cancelled) {
        Logger::global().info(QStringLiteral("Process cancelled"), QStringLiteral("runtime"));
    }

    if (entry.restartRequested) {
        it->restartRequested = false;
        startProcess(id);
        emit processRestarted(id, entry.restartCount + 1);
        return;
    }

    if (shouldRestart(entry, state)) {
        scheduleRestart(id);
        return;
    }

    if (it->process) {
        it->process->deleteLater();
    }
    processes_.erase(it);
}

void ProcessManager::scheduleRestart(const QUuid& id) {
    auto it = processes_.find(id);
    if (it == processes_.end()) {
        return;
    }
    ProcessEntry& entry = it.value();
    entry.restartCount += 1;
    emit processRestarted(id, entry.restartCount);
    Logger::global().warning(QStringLiteral("Restarting process (%1/%2)")
                                 .arg(entry.restartCount)
                                 .arg(entry.spec.restartPolicy.maxRetries),
                             QStringLiteral("runtime"));

    QTimer::singleShot(entry.spec.restartPolicy.delayMs, this, [this, id]() { startProcess(id); });
}

bool ProcessManager::shouldRestart(const ProcessEntry& entry, const ProcessExitState& state) const {
    if (entry.spec.restartPolicy.mode == RestartMode::Never) {
        return false;
    }
    if (entry.spec.restartPolicy.maxRetries > 0 && entry.restartCount >= entry.spec.restartPolicy.maxRetries) {
        return false;
    }
    if (entry.spec.restartPolicy.mode == RestartMode::Always) {
        return true;
    }
    if (entry.spec.restartPolicy.mode == RestartMode::OnFailure) {
        return state.reason == ProcessExitReason::Crashed || state.reason == ProcessExitReason::FailedToStart ||
               (state.reason == ProcessExitReason::Exited && state.exitCode != 0);
    }
    return false;
}
}  // namespace webide
