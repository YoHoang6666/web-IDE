#include "TerminalSession.h"

#include "core/Logger.h"

namespace webide {
TerminalSession::TerminalSession(QObject* parent)
    : QObject(parent), processManager_(new ProcessManager(this)), runtimeSession_(new RuntimeSession(this)), shell_(ShellDetector::detectDefault()) {
    connect(processManager_, &ProcessManager::processStarted, this, [this](const QUuid& id, qint64 pid) {
        if (id != activeProcessId_) {
            return;
        }
        activePid_ = pid;
        runtimeSession_->registerProcess(pid);
        emit statusMessage(tr("Process started (%1)").arg(pid));
        emit runningChanged(true);
    });

    connect(processManager_, &ProcessManager::processOutput, this, [this](const QUuid& id, const QString& text, bool isStdErr) {
        if (id == activeProcessId_) {
            emit outputReady(text, isStdErr);
        }
    });

    connect(processManager_, &ProcessManager::processExited, this, [this](const QUuid& id, const ProcessExitState& state) {
        if (id != activeProcessId_) {
            return;
        }
        if (activePid_ != 0) {
            runtimeSession_->unregisterProcess(activePid_);
            activePid_ = 0;
        }
        running_ = false;
        activeProcessId_ = {};
        emit processExited(state);
        emit runningChanged(false);
        emit statusMessage(tr("Process %1 (code %2)").arg(state.message).arg(state.exitCode));
    });

    connect(processManager_, &ProcessManager::processRestarted, this, [this](const QUuid& id, int attempt) {
        if (id == activeProcessId_) {
            emit processRestarted(attempt);
            emit statusMessage(tr("Process restarted (attempt %1)").arg(attempt));
        }
    });
}

bool TerminalSession::runCommand(const QString& command, const TerminalCommandOptions& options) {
    if (command.trimmed().isEmpty()) {
        return false;
    }
    if (running_) {
        emit statusMessage(tr("A command is already running"));
        return false;
    }

    shell_ = ShellDetector::detectDefault();

    ProcessSpec spec;
    spec.program = shell_.program;
    spec.arguments = shell_.arguments;
    spec.arguments << command;
    spec.workingDirectory = runtimeSession_->workingDirectory();
    spec.environment = runtimeSession_->environment();
    spec.detached = options.detached;
    spec.restartPolicy = options.restartPolicy;
    spec.displayName = command;

    activeProcessId_ = processManager_->start(spec);
    running_ = !spec.detached;

    if (spec.detached) {
        emit statusMessage(tr("Detached process started: %1").arg(command));
        running_ = false;
    } else {
        emit statusMessage(tr("Running: %1").arg(command));
    }
    return true;
}

void TerminalSession::cancelActive() {
    if (activeProcessId_.isNull()) {
        emit statusMessage(tr("No active process to cancel"));
        return;
    }
    processManager_->cancel(activeProcessId_);
    emit statusMessage(tr("Cancellation requested"));
}

bool TerminalSession::isRunning() const { return running_; }

void TerminalSession::setWorkingDirectory(const QString& path) {
    runtimeSession_->setWorkingDirectory(path);
}

void TerminalSession::setEnvironment(const QProcessEnvironment& environment) {
    runtimeSession_->setEnvironment(environment);
}

ShellDescriptor TerminalSession::shell() const { return shell_; }

RuntimeSession* TerminalSession::runtimeSession() const { return runtimeSession_; }
}  // namespace webide
