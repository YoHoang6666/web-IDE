#include "PythonRuntime.h"

namespace webide {
PythonRuntime::PythonRuntime(QObject* parent)
    : RuntimeBase(QStringLiteral("python"), QStringLiteral("Python"), parent) {}

QStringList PythonRuntime::candidateExecutables() const {
    return {QStringLiteral("python"), QStringLiteral("python3")};
}
}  // namespace webide

