#pragma once

#include <QWidget>

class QListWidget;
class QPlainTextEdit;

namespace webide {
class RuntimeManager;
class ServerControlWidget;
class ProcessMonitorWidget;
class RuntimeSettingsWidget;

class RuntimePanel : public QWidget {
    Q_OBJECT

public:
    explicit RuntimePanel(RuntimeManager* runtimeManager, QWidget* parent = nullptr);

signals:
    void statusMessage(const QString& message);

private:
    void setupUi();
    void wireSignals();
    void refreshRuntimeList();
    void refreshProcessList();

    RuntimeManager* runtimeManager_;
    QListWidget* runtimesList_;
    ServerControlWidget* serverControl_;
    ProcessMonitorWidget* processMonitor_;
    RuntimeSettingsWidget* runtimeSettings_;
    QPlainTextEdit* logsOutput_;
    QString activeProcessId_;
};
}  // namespace webide

