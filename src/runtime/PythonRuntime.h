#pragma once

#include "RuntimeBase.h"

namespace webide {
class PythonRuntime final : public RuntimeBase {
    Q_OBJECT

public:
    explicit PythonRuntime(QObject* parent = nullptr);

protected:
    QStringList candidateExecutables() const override;
};
}  // namespace webide

