#include "SecurityPolicy.h"

namespace webide {
bool SecurityPolicy::allowWorkspaceAccess(const QString& path) const { return !path.trimmed().isEmpty(); }
}  // namespace webide
