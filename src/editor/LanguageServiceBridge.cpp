#include "LanguageServiceBridge.h"

namespace webide {
LanguageServiceBridge::LanguageServiceBridge(QObject* parent) : QObject(parent) {}

QStringList LanguageServiceBridge::completionsFor(const QString& path, int line, int column) const {
    Q_UNUSED(path);
    Q_UNUSED(line);
    Q_UNUSED(column);
    return {QStringLiteral("div"), QStringLiteral("class"), QStringLiteral("SELECT")};
}
}  // namespace webide
