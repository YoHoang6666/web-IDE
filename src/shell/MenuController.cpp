#include "MenuController.h"

#include <QAction>
#include <QMainWindow>
#include <QMenuBar>

namespace webide {
MenuController::MenuController(QMainWindow* window) : QObject(window), window_(window) {}

void MenuController::buildMenus() {
    auto* fileMenu = window_->menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(new QAction(tr("Open Workspace"), this));
    fileMenu->addAction(new QAction(tr("Save All"), this));

    auto* viewMenu = window_->menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(new QAction(tr("Editor / Preview"), this));
    viewMenu->addAction(new QAction(tr("Editor / Database"), this));

    auto* toolsMenu = window_->menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(new QAction(tr("Inspect Element"), this));
    toolsMenu->addAction(new QAction(tr("Console"), this));
    toolsMenu->addAction(new QAction(tr("Network"), this));
}
}  // namespace webide
