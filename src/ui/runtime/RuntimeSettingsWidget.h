#pragma once

#include <QWidget>

class QLineEdit;

namespace webide {
class RuntimeSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit RuntimeSettingsWidget(QWidget* parent = nullptr);

    void setPhpPath(const QString& path);
    void setNodePath(const QString& path);
    void setPythonPath(const QString& path);

private:
    QLineEdit* phpPath_;
    QLineEdit* nodePath_;
    QLineEdit* pythonPath_;
};
}  // namespace webide
