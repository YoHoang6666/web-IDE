#include "RuntimeSession.h"

namespace webide {
RuntimeSession::RuntimeSession(QObject* parent) : QObject(parent), environment_(QProcessEnvironment::systemEnvironment()) {}

void RuntimeSession::setWorkingDirectory(const QString& path) {
    if (workingDirectory_ == path) {
        return;
    }
    workingDirectory_ = path;
    emit workingDirectoryChanged(path);
}

QString RuntimeSession::workingDirectory() const { return workingDirectory_; }

void RuntimeSession::setEnvironment(const QProcessEnvironment& environment) {
    environment_ = environment;
    emit environmentChanged();
}

QProcessEnvironment RuntimeSession::environment() const { return environment_; }

void RuntimeSession::assignPort(int port) {
    if (!assignedPorts_.contains(port)) {
        assignedPorts_.append(port);
        emit portAssigned(port);
    }
}

QVector<int> RuntimeSession::assignedPorts() const { return assignedPorts_; }

void RuntimeSession::registerProcess(qint64 pid) {
    if (!processTree_.contains(pid)) {
        processTree_.append(pid);
        emit processRegistered(pid);
    }
}

void RuntimeSession::unregisterProcess(qint64 pid) {
    if (processTree_.removeOne(pid)) {
        emit processRemoved(pid);
    }
}

QVector<qint64> RuntimeSession::processTree() const { return processTree_; }
}  // namespace webide
