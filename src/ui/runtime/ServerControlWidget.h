#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace webide {
class ServerControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ServerControlWidget(QWidget* parent = nullptr);

signals:
    void startRequested(const QString& runtimeId, const QString& command, int port);
    void stopRequested();
    void restartRequested();
    void killRequested();
    void openBrowserRequested(int port);
    void openTerminalRequested(const QString& command);

private:
    QComboBox* runtimeBox_;
    QLineEdit* commandEdit_;
    QSpinBox* portSpin_;
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QPushButton* restartButton_;
    QPushButton* killButton_;
    QPushButton* openBrowserButton_;
    QPushButton* openTerminalButton_;
};
}  // namespace webide

