#include "LocalApiBridge.h"

namespace webide {
LocalApiBridge::LocalApiBridge(QObject* parent) : QObject(parent) {}

QVariant LocalApiBridge::invoke(const QString& method, const QVariantMap& payload) {
    QVariantMap response = payload;
    response.insert(QStringLiteral("method"), method);
    response.insert(QStringLiteral("transport"), QStringLiteral("local-runtime"));
    return response;
}
}  // namespace webide
