#pragma once

#include <QWidget>

namespace webide {
class MonacoWebViewBridge : public QWidget {
    Q_OBJECT

public:
    explicit MonacoWebViewBridge(QWidget* parent = nullptr);
};
}  // namespace webide
