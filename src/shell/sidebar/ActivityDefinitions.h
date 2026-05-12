#pragma once

#include <QString>

namespace webide {
enum class ActivityId {
    Explorer,
    Search,
    Git,
    Database,
    Runtime,
    Preview,
    Network,
    DevTools,
    Extensions,
    Settings
};

struct ActivityDefinition {
    ActivityId id;
    QString iconText;
    QString title;
    QString shortcut;
};
}  // namespace webide

