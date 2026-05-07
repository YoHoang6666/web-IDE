#pragma once

#include <QString>

namespace webide {
class SecurityPolicy {
public:
    bool allowWorkspaceAccess(const QString& path) const;
};
}  // namespace webide
