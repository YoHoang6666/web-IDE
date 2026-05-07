#pragma once

#include <QWidget>

namespace webide {
class CEFHostWidget;

class PreviewPane : public QWidget {
    Q_OBJECT

public:
    explicit PreviewPane(QWidget* parent = nullptr);

    void loadTarget(const QString& target);
    CEFHostWidget* browserHost() const;

signals:
    void previewTargetChanged(const QString& target);

private:
    CEFHostWidget* browserHost_;
};
}  // namespace webide
