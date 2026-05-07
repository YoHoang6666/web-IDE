#include "ConflictResolver.h"

namespace webide {
ChangeSet ConflictResolver::resolve(const ChangeSet& left, const ChangeSet& right) const {
    return left.observedAt >= right.observedAt ? left : right;
}
}  // namespace webide
