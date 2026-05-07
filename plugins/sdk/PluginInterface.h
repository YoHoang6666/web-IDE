#pragma once

#include <QString>

namespace webide {
class PluginInterface {
public:
    virtual ~PluginInterface() = default;
    virtual QString pluginId() const = 0;
    virtual void registerModule() = 0;
};
}  // namespace webide
