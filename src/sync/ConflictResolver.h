#pragma once

#include "ChangeSet.h"

namespace webide {
class ConflictResolver {
public:
    ChangeSet resolve(const ChangeSet& left, const ChangeSet& right) const;
};
}  // namespace webide
