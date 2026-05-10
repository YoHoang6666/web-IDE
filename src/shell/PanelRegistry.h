#pragma once

#include <QHash>
#include <QObject>
#include <QString>

#include <functional>

class QAction;
class QDockWidget;
class QMainWindow;
class QSettings;
class QWidget;

namespace webide {
struct PanelDefinition {
    QString id;
    QString title;
    Qt::DockWidgetArea defaultArea = Qt::LeftDockWidgetArea;
    bool visibleByDefault = true;
    QString source;
    std::function<QWidget*(QWidget* parent)> factory;
};

class PanelRegistry : public QObject {
    Q_OBJECT

public:
    explicit PanelRegistry(QMainWindow* window, QObject* parent = nullptr);

    void registerPanel(const PanelDefinition& definition);
    QStringList panelIds() const;

    QDockWidget* dockWidget(const QString& id) const;
    QWidget* panelWidget(const QString& id) const;
    QWidget* ensurePanelWidget(const QString& id);
    QAction* toggleAction(const QString& id) const;

    void restoreState(QSettings* settings);
    void saveState(QSettings* settings) const;

signals:
    void panelReady(const QString& id, QWidget* widget);

private:
    struct PanelEntry {
        PanelDefinition definition;
        QDockWidget* dock = nullptr;
        QWidget* placeholder = nullptr;
        QWidget* widget = nullptr;
        QAction* toggle = nullptr;
        bool initialized = false;
    };

    void ensureWidget(PanelEntry& entry);

    QMainWindow* window_;
    QHash<QString, PanelEntry> panels_;
};
}  // namespace webide
