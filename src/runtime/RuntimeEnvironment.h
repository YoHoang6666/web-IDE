#pragma once

#include <QString>

namespace webide {
struct RuntimeEnvironment {
    QString id;
    QString displayName;
    QString executablePath;
    QString version;
    bool available = false;
};
}  // namespace webide

