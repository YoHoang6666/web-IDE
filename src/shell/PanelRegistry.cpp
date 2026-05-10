#include "PanelRegistry.h"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QSettings>

#include "core/Logger.h"

namespace webide {
PanelRegistry::PanelRegistry(QMainWindow* window, QObject* parent) : QObject(parent), window_(window) {}

void PanelRegistry::registerPanel(const PanelDefinition& definition) {
    if (!window_ || definition.id.isEmpty() || panels_.contains(definition.id)) {
        Logger::global().warning(QStringLiteral("Panel registration skipped: %1").arg(definition.id),
                                 QStringLiteral("panel"));
        return;
    }

    PanelEntry entry;
    entry.definition = definition;

    entry.dock = new QDockWidget(definition.title, window_);
    entry.dock->setObjectName(definition.id);
    entry.dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    entry.dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);

    entry.placeholder = new QLabel(tr("Loading %1...").arg(definition.title), entry.dock);
    entry.placeholder->setObjectName(QStringLiteral("panel-placeholder-%1").arg(definition.id));
    entry.dock->setWidget(entry.placeholder);

    entry.toggle = entry.dock->toggleViewAction();
    entry.toggle->setText(definition.title);

    connect(entry.dock, &QDockWidget::visibilityChanged, this, [this, id = definition.id](bool visible) {
        if (!visible) {
            return;
        }
        auto it = panels_.find(id);
        if (it != panels_.end()) {
            ensureWidget(it.value());
        }
    });

    window_->addDockWidget(definition.defaultArea, entry.dock);
    entry.dock->setVisible(definition.visibleByDefault);

    panels_.insert(definition.id, entry);
}

QStringList PanelRegistry::panelIds() const { return panels_.keys(); }

QDockWidget* PanelRegistry::dockWidget(const QString& id) const {
    return panels_.contains(id) ? panels_.value(id).dock : nullptr;
}

QWidget* PanelRegistry::panelWidget(const QString& id) const {
    return panels_.contains(id) ? panels_.value(id).widget : nullptr;
}

QWidget* PanelRegistry::ensurePanelWidget(const QString& id) {
    auto it = panels_.find(id);
    if (it == panels_.end()) {
        return nullptr;
    }
    ensureWidget(it.value());
    return it.value().widget;
}

QAction* PanelRegistry::toggleAction(const QString& id) const {
    return panels_.contains(id) ? panels_.value(id).toggle : nullptr;
}

void PanelRegistry::restoreState(QSettings* settings) {
    if (!settings || !window_) {
        return;
    }

    const QByteArray state = settings->value(QStringLiteral("window/state")).toByteArray();
    if (!state.isEmpty()) {
        window_->restoreState(state);
    }

    for (auto it = panels_.begin(); it != panels_.end(); ++it) {
        const QString key = QStringLiteral("panels/%1/visible").arg(it.key());
        const bool visible = settings->value(key, it->definition.visibleByDefault).toBool();
        if (it->dock) {
            it->dock->setVisible(visible);
            if (visible) {
                ensureWidget(it.value());
            }
        }
    }
}

void PanelRegistry::saveState(QSettings* settings) const {
    if (!settings || !window_) {
        return;
    }

    settings->setValue(QStringLiteral("window/state"), window_->saveState());
    for (auto it = panels_.begin(); it != panels_.end(); ++it) {
        if (!it->dock) {
            continue;
        }
        const QString key = QStringLiteral("panels/%1/visible").arg(it.key());
        settings->setValue(key, it->dock->isVisible());
    }
}

void PanelRegistry::ensureWidget(PanelEntry& entry) {
    if (entry.initialized) {
        return;
    }
    if (!entry.definition.factory) {
        return;
    }

    entry.widget = entry.definition.factory(entry.dock);
    if (!entry.widget) {
        Logger::global().warning(QStringLiteral("Panel factory returned null: %1").arg(entry.definition.id),
                                 QStringLiteral("panel"));
        return;
    }

    entry.widget->setParent(entry.dock);
    entry.dock->setWidget(entry.widget);
    entry.initialized = true;
    emit panelReady(entry.definition.id, entry.widget);
}
}  // namespace webide
