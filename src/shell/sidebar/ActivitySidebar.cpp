#include "ActivitySidebar.h"

#include <QSizePolicy>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace webide {
ActivitySidebar::ActivitySidebar(QWidget* parent) : QWidget(parent), layout_(new QVBoxLayout(this)) {
    layout_->setContentsMargins(4, 6, 4, 6);
    layout_->setSpacing(4);
    setObjectName(QStringLiteral("activitySidebar"));
    setFixedWidth(48);
}

void ActivitySidebar::setActivities(const QList<ActivityDefinition>& activities) {
    QLayoutItem* item = nullptr;
    while ((item = layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    buttons_.clear();

    for (const ActivityDefinition& activity : activities) {
        auto* button = new QToolButton(this);
        button->setText(activity.iconText);
        button->setToolTip(QStringLiteral("%1 (%2)").arg(activity.title, activity.shortcut));
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setObjectName(QStringLiteral("activityButton"));
        button->setProperty("activityActive", false);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setMinimumHeight(32);
        connect(button, &QToolButton::clicked, this, [this, activity]() {
            emit activityTriggered(activity.id);
        });
        layout_->addWidget(button);
        buttons_.insert(static_cast<int>(activity.id), button);
    }

    layout_->addStretch();

    auto* collapseButton = new QToolButton(this);
    collapseButton->setText(QStringLiteral("≡"));
    collapseButton->setToolTip(tr("Collapse sidebar"));
    collapseButton->setObjectName(QStringLiteral("activityButton"));
    connect(collapseButton, &QToolButton::clicked, this, &ActivitySidebar::collapseRequested);
    layout_->addWidget(collapseButton);
}

void ActivitySidebar::setActiveActivity(ActivityId activity) {
    for (auto it = buttons_.begin(); it != buttons_.end(); ++it) {
        const bool active = (it.key() == static_cast<int>(activity));
        it.value()->setChecked(active);
        it.value()->setProperty("activityActive", active);
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
        it.value()->update();
    }
}
}  // namespace webide
