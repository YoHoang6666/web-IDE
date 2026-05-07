#pragma once

#include <QObject>

class QStatusBar;
class QLabel;

namespace webide {
class StatusBarController : public QObject {
    Q_OBJECT

public:
    explicit StatusBarController(QStatusBar* statusBar, QObject* parent = nullptr);

    void showWorkspaceMessage(const QString& message);
    void setMode(const QString& mode);

private:
    QStatusBar* statusBar_;
    QLabel* modeLabel_;
};
}  // namespace webide
