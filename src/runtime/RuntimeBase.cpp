#include "RuntimeBase.h"

#include <QProcess>
#include <QStandardPaths>
#include <utility>

namespace webide {
namespace {
constexpr int kVersionCheckTimeoutMs = 2000;
constexpr int kKillWaitTimeoutMs = 300;
}  // namespace

RuntimeBase::RuntimeBase(QString id, QString displayName, QObject* parent)
    : QObject(parent), id_(std::move(id)), displayName_(std::move(displayName)) {
    environment_.id = id_;
    environment_.displayName = displayName_;
}

QString RuntimeBase::id() const { return id_; }

QString RuntimeBase::displayName() const { return displayName_; }

RuntimeEnvironment RuntimeBase::environment() const { return environment_; }

void RuntimeBase::detect() {
    const QString executablePath = findExecutable(candidateExecutables());
    environment_ = buildEnvironment(executablePath);
    environment_.id = id_;
    environment_.displayName = displayName_;
    emit environmentChanged(environment_);
}

QString RuntimeBase::versionCommand() const { return QStringLiteral("--version"); }

RuntimeEnvironment RuntimeBase::buildEnvironment(const QString& executablePath) const {
    RuntimeEnvironment environment;
    environment.id = id_;
    environment.displayName = displayName_;
    environment.executablePath = executablePath;
    environment.available = !executablePath.isEmpty();
    if (!executablePath.isEmpty()) {
        environment.version = readFirstLineFromCommand(executablePath, versionCommand());
    }
    return environment;
}

QString RuntimeBase::findExecutable(const QStringList& names) {
    for (const QString& name : names) {
        const QString resolved = QStandardPaths::findExecutable(name);
        if (!resolved.isEmpty()) {
            return resolved;
        }
    }
    return {};
}

QString RuntimeBase::readFirstLineFromCommand(const QString& executablePath, const QString& command) {
    if (executablePath.isEmpty()) {
        return {};
    }

    QProcess process;
    process.start(executablePath, {command});
    if (!process.waitForFinished(kVersionCheckTimeoutMs)) {
        process.kill();
        process.waitForFinished(kKillWaitTimeoutMs);
        return {};
    }

    const QString stdoutText = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    if (!stdoutText.isEmpty()) {
        return stdoutText.section('\n', 0, 0).trimmed();
    }
    const QString stderrText = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    return stderrText.section('\n', 0, 0).trimmed();
}
}  // namespace webide
