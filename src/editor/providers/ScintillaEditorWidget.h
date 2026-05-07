#pragma once

#include <QWidget>

namespace webide {
class ScintillaEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScintillaEditorWidget(QWidget* parent = nullptr);
};
}  // namespace webide
