#pragma once

#include <QObject>
#include <QStringList>

#include "RuntimeEnvironment.h"

namespace webide {
class RuntimeBase : public QObject {
    Q_OBJECT

public:
    explicit RuntimeBase(QString id, QString displayName, QObject* parent = nullptr);
    ~RuntimeBase() override = default;

    QString id() const;
    QString displayName() const;
    RuntimeEnvironment environment() const;

public slots:
    virtual void detect();

signals:
    void environmentChanged(const webide::RuntimeEnvironment& environment);

protected:
    virtual QStringList candidateExecutables() const = 0;
    virtual QString versionCommand() const;
    virtual RuntimeEnvironment buildEnvironment(const QString& executablePath) const;

    static QString findExecutable(const QStringList& names);
    static QString readFirstLineFromCommand(const QString& executablePath, const QString& command);

private:
    QString id_;
    QString displayName_;
    RuntimeEnvironment environment_;
};
}  // namespace webide
