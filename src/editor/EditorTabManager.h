#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace webide {
class EditorTabManager : public QObject {
    Q_OBJECT

public:
    explicit EditorTabManager(QObject* parent = nullptr);

    void openTab(const QString& path);
    void closeTab(const QString& path);
    QStringList openTabs() const;

signals:
    void tabsChanged();

private:
    QStringList openTabs_;
};
}  // namespace webide
