#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantMap>

namespace webide {
class LocalApiBridge : public QObject {
    Q_OBJECT

public:
    explicit LocalApiBridge(QObject* parent = nullptr);
    QVariant invoke(const QString& method, const QVariantMap& payload = {});
};
}  // namespace webide
