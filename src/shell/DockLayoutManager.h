#pragma once

#include <QByteArray>
#include <QObject>

class QMainWindow;

namespace webide {
class DockLayoutManager : public QObject {
    Q_OBJECT

public:
    explicit DockLayoutManager(QMainWindow* window);

    void captureDefaultState(const QByteArray& state);
    void restoreDefaultLayout();
    void applyEditorPreviewLayout();
    void applyEditorDatabaseLayout();

private:
    QMainWindow* window_;
    QByteArray defaultState_;
};
}  // namespace webide
