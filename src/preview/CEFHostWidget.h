#pragma once

#include <QWidget>

class QLabel;

namespace webide {
class CEFHostWidget : public QWidget {
    Q_OBJECT

public:
    explicit CEFHostWidget(QWidget* parent = nullptr);
    void navigate(const QString& target);

private:
    QLabel* stateLabel_;
};
}  // namespace webide
