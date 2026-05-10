#pragma once

#include <QObject>
#include <QProcessEnvironment>
#include <QString>
#include <QVector>

namespace webide {
class RuntimeSession : public QObject {
    Q_OBJECT

public:
    explicit RuntimeSession(QObject* parent = nullptr);

    void setWorkingDirectory(const QString& path);
    QString workingDirectory() const;

    void setEnvironment(const QProcessEnvironment& environment);
    QProcessEnvironment environment() const;

    void assignPort(int port);
    QVector<int> assignedPorts() const;

    void registerProcess(qint64 pid);
    void unregisterProcess(qint64 pid);
    QVector<qint64> processTree() const;

signals:
    void workingDirectoryChanged(const QString& path);
    void environmentChanged();
    void portAssigned(int port);
    void processRegistered(qint64 pid);
    void processRemoved(qint64 pid);

private:
    QString workingDirectory_;
    QProcessEnvironment environment_;
    QVector<int> assignedPorts_;
    QVector<qint64> processTree_;
};
}  // namespace webide
