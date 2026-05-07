#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace webide {
class LanguageServiceBridge : public QObject {
    Q_OBJECT

public:
    explicit LanguageServiceBridge(QObject* parent = nullptr);
    QStringList completionsFor(const QString& path, int line, int column) const;
};
}  // namespace webide
