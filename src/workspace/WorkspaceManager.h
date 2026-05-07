#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace webide {
class WorkspaceManager : public QObject {
    Q_OBJECT

public:
    explicit WorkspaceManager(QObject* parent = nullptr);

    void openWorkspace(const QString& path);
    QString currentWorkspace() const;
    QStringList recentWorkspaces() const;

signals:
    void workspaceOpened(const QString& path);

private:
    QString currentWorkspace_;
    QStringList recentWorkspaces_;
};
}  // namespace webide
