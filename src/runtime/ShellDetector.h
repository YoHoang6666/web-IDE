#pragma once

#include <QString>
#include <QStringList>

namespace webide {
struct ShellDescriptor {
    QString name;
    QString program;
    QStringList arguments;
    QString encoding = QStringLiteral("UTF-8");
    bool supportsAnsi = true;
};

class ShellDetector {
public:
    static ShellDescriptor detectDefault();
};
}  // namespace webide
