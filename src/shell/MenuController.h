#pragma once

#include <QObject>

class QMainWindow;

namespace webide {
class MenuController : public QObject {
    Q_OBJECT

public:
    explicit MenuController(QMainWindow* window);

    void buildMenus();

private:
    QMainWindow* window_;
};
}  // namespace webide
