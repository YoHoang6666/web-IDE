#pragma once

#include "RuntimeBase.h"

namespace webide {
class PhpRuntime final : public RuntimeBase {
    Q_OBJECT

public:
    explicit PhpRuntime(QObject* parent = nullptr);

protected:
    QStringList candidateExecutables() const override;
    QString versionCommand() const override;
};
}  // namespace webide

