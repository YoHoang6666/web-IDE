#include "PhpRuntime.h"

namespace webide {
PhpRuntime::PhpRuntime(QObject* parent)
    : RuntimeBase(QStringLiteral("php"), QStringLiteral("PHP"), parent) {}

QStringList PhpRuntime::candidateExecutables() const {
    return {QStringLiteral("php")};
}

QString PhpRuntime::versionCommand() const { return QStringLiteral("-v"); }
}  // namespace webide

