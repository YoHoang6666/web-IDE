#pragma once

#include <QHash>
#include <QWidget>

#include "ActivityDefinitions.h"

class QToolButton;
class QVBoxLayout;

namespace webide {
class ActivitySidebar : public QWidget {
    Q_OBJECT

public:
    explicit ActivitySidebar(QWidget* parent = nullptr);

    void setActivities(const QList<ActivityDefinition>& activities);
    void setActiveActivity(ActivityId activity);

signals:
    void activityTriggered(webide::ActivityId activity);
    void collapseRequested();

private:
    QVBoxLayout* layout_;
    QHash<int, QToolButton*> buttons_;
};
}  // namespace webide
