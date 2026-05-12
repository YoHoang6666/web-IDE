#pragma once

#include <QWidget>

#include "runtime/ProcessManager.h"

class QTableWidget;

namespace webide {
class ProcessMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProcessMonitorWidget(QWidget* parent = nullptr);
    void setProcesses(const QList<ManagedProcess>& processes);
    QString selectedProcessId() const;

private:
    QTableWidget* table_;
};
}  // namespace webide

