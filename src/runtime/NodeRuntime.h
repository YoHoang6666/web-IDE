#pragma once

#include "RuntimeBase.h"

namespace webide {
class NodeRuntime final : public RuntimeBase {
    Q_OBJECT

public:
    explicit NodeRuntime(QObject* parent = nullptr);

protected:
    QStringList candidateExecutables() const override;
};
}  // namespace webide

