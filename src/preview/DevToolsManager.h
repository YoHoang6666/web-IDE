#pragma once

#include <QObject>

namespace webide {
class PreviewPane;

class DevToolsManager : public QObject {
    Q_OBJECT

public:
    explicit DevToolsManager(QObject* parent = nullptr);

    void bindPreviewPane(PreviewPane* previewPane);
    void openInspector();

private:
    PreviewPane* previewPane_ = nullptr;
};
}  // namespace webide
