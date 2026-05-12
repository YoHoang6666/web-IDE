#include "ActivityPanelHost.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace webide {
ActivityPanelHost::ActivityPanelHost(QWidget* parent)
    : QWidget(parent),
      titleLabel_(new QLabel(this)),
      collapseButton_(new QToolButton(this)),
      stacked_(new QStackedWidget(this)) {
    setObjectName(QStringLiteral("activityPanelHost"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* header = new QWidget(this);
    header->setObjectName(QStringLiteral("activityPanelHeader"));
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 6, 6, 6);
    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();
    collapseButton_->setText(QStringLiteral("×"));
    collapseButton_->setToolTip(tr("Collapse"));
    headerLayout->addWidget(collapseButton_);

    rootLayout->addWidget(header);
    rootLayout->addWidget(stacked_, 1);

    connect(collapseButton_, &QToolButton::clicked, this, &ActivityPanelHost::collapseRequested);
}

void ActivityPanelHost::registerPanel(ActivityId id, const QString& title, QWidget* panel) {
    if (!panel) {
        return;
    }
    panel->setParent(stacked_);
    const int index = stacked_->addWidget(panel);
    panelIndex_.insert(static_cast<int>(id), index);
    titles_.insert(static_cast<int>(id), title);
}

void ActivityPanelHost::setCurrentActivity(ActivityId id) {
    const int key = static_cast<int>(id);
    const int index = panelIndex_.value(key, -1);
    if (index < 0) {
        return;
    }
    currentActivity_ = id;
    stacked_->setCurrentIndex(index);
    titleLabel_->setText(titles_.value(key));
}

ActivityId ActivityPanelHost::currentActivity() const { return currentActivity_; }
}  // namespace webide
