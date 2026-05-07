#pragma once

#include <QDebug>
#include <QString>

namespace webide {
class Logger {
public:
    void info(const QString& message) const { qInfo().noquote() << message; }
    void warning(const QString& message) const { qWarning().noquote() << message; }
    void error(const QString& message) const { qCritical().noquote() << message; }
};
}  // namespace webide
