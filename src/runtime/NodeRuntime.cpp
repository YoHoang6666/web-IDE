#include "NodeRuntime.h"

namespace webide {
NodeRuntime::NodeRuntime(QObject* parent)
    : RuntimeBase(QStringLiteral("node"), QStringLiteral("Node.js"), parent) {}

QStringList NodeRuntime::candidateExecutables() const {
    return {QStringLiteral("node"), QStringLiteral("nodejs")};
}
}  // namespace webide

