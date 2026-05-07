#pragma once

#include <QObject>
#include <QString>

namespace webide {
class ScriptExecutionContext : public QObject {
    Q_OBJECT

public:
    explicit ScriptExecutionContext(QObject* parent = nullptr);

    void setWorkspacePath(const QString& workspacePath);
    QString workspacePath() const;

private:
    QString workspacePath_;
};
}  // namespace webide
