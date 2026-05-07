#pragma once

#include <QWidget>

class QLabel;
class QSplitter;

namespace webide {
class EditorHost : public QWidget {
    Q_OBJECT

public:
    explicit EditorHost(QWidget* parent = nullptr);
    void enableSecondarySplit(bool enabled);

private:
    QSplitter* splitter_;
    QLabel* primaryEditor_;
    QLabel* secondaryEditor_;
};
}  // namespace webide
