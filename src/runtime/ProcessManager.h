#pragma once

#include <QObject>
#include <QProcess>
#include <QHash>

namespace webide {
struct ManagedProcess {
    QString id;
    QString runtimeId;
    QString commandLabel;
    QString program;
    QStringList arguments;
    QString workingDirectory;
    int port = 0;
    QProcess::ProcessState state = QProcess::NotRunning;
    int exitCode = 0;
};

class ProcessManager : public QObject {
    Q_OBJECT

public:
    explicit ProcessManager(QObject* parent = nullptr);

    QString startProcess(const QString& runtimeId,
                         const QString& commandLabel,
                         const QString& program,
                         const QStringList& arguments,
                         const QString& workingDirectory = {},
                         int port = 0);
    bool stopProcess(const QString& processId);
    bool killProcess(const QString& processId);
    QList<ManagedProcess> processes() const;

signals:
    void processUpdated(const webide::ManagedProcess& process);
    void processOutput(const QString& processId, const QString& text, bool isError);
    void processFinished(const QString& processId, int exitCode, QProcess::ExitStatus status);

private:
    struct ProcessEntry {
        ManagedProcess meta;
        QProcess* process = nullptr;
    };

    static QString nextId();
    void emitUpdate(const ProcessEntry& entry);

    QHash<QString, ProcessEntry> processes_;
};
}  // namespace webide

