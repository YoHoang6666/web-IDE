#include "ProcessManager.h"

#include <QDateTime>
#include <atomic>

namespace webide {
ProcessManager::ProcessManager(QObject* parent) : QObject(parent) {}

QString ProcessManager::nextId() {
    static std::atomic<int> counter{0};
    return QStringLiteral("proc-%1-%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(++counter);
}

QString ProcessManager::startProcess(const QString& runtimeId,
                                     const QString& commandLabel,
                                     const QString& program,
                                     const QStringList& arguments,
                                     const QString& workingDirectory,
                                     int port) {
    if (program.isEmpty()) {
        return {};
    }

    ProcessEntry entry;
    entry.meta.id = nextId();
    entry.meta.runtimeId = runtimeId;
    entry.meta.commandLabel = commandLabel;
    entry.meta.program = program;
    entry.meta.arguments = arguments;
    entry.meta.workingDirectory = workingDirectory;
    entry.meta.port = port;
    entry.meta.state = QProcess::Starting;
    entry.process = new QProcess(this);

    if (!workingDirectory.isEmpty()) {
        entry.process->setWorkingDirectory(workingDirectory);
    }

    const QString processId = entry.meta.id;
    connect(entry.process, &QProcess::readyReadStandardOutput, this, [this, processId]() {
        auto it = processes_.find(processId);
        if (it == processes_.end()) {
            return;
        }
        emit processOutput(processId, QString::fromLocal8Bit(it->process->readAllStandardOutput()), false);
    });
    connect(entry.process, &QProcess::readyReadStandardError, this, [this, processId]() {
        auto it = processes_.find(processId);
        if (it == processes_.end()) {
            return;
        }
        emit processOutput(processId, QString::fromLocal8Bit(it->process->readAllStandardError()), true);
    });
    connect(entry.process, &QProcess::stateChanged, this, [this, processId](QProcess::ProcessState state) {
        auto it = processes_.find(processId);
        if (it == processes_.end()) {
            return;
        }
        it->meta.state = state;
        emitUpdate(*it);
    });
    connect(entry.process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this, processId](int exitCode, QProcess::ExitStatus status) {
                auto it = processes_.find(processId);
                if (it == processes_.end()) {
                    return;
                }
                it->meta.exitCode = exitCode;
                it->meta.state = QProcess::NotRunning;
                emitUpdate(*it);
                emit processFinished(processId, exitCode, status);
            });

    processes_.insert(processId, entry);
    auto it = processes_.find(processId);
    it->process->start(program, arguments);
    emitUpdate(*it);
    return processId;
}

bool ProcessManager::stopProcess(const QString& processId) {
    auto it = processes_.find(processId);
    if (it == processes_.end() || !it->process) {
        return false;
    }
    it->process->terminate();
    return true;
}

bool ProcessManager::killProcess(const QString& processId) {
    auto it = processes_.find(processId);
    if (it == processes_.end() || !it->process) {
        return false;
    }
    it->process->kill();
    return true;
}

QList<ManagedProcess> ProcessManager::processes() const {
    QList<ManagedProcess> result;
    result.reserve(processes_.size());
    for (const ProcessEntry& entry : processes_) {
        result.push_back(entry.meta);
    }
    return result;
}

void ProcessManager::emitUpdate(const ProcessEntry& entry) { emit processUpdated(entry.meta); }
}  // namespace webide
