#pragma once

#include <QHash>
#include <QWidget>

#include "ActivityDefinitions.h"

class QLabel;
class QStackedWidget;
class QToolButton;

namespace webide {
class ActivityPanelHost : public QWidget {
    Q_OBJECT

public:
    explicit ActivityPanelHost(QWidget* parent = nullptr);

    void registerPanel(ActivityId id, const QString& title, QWidget* panel);
    void setCurrentActivity(ActivityId id);
    ActivityId currentActivity() const;

signals:
    void collapseRequested();

private:
    QLabel* titleLabel_;
    QToolButton* collapseButton_;
    QStackedWidget* stacked_;
    QHash<int, int> panelIndex_;
    QHash<int, QString> titles_;
    ActivityId currentActivity_ = ActivityId::Explorer;
};
}  // namespace webide
