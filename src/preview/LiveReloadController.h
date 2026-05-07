#pragma once

#include <QObject>

#include "sync/ChangeSet.h"

namespace webide {
class PreviewPane;

class LiveReloadController : public QObject {
    Q_OBJECT

public:
    explicit LiveReloadController(QObject* parent = nullptr);

    void bindPreviewPane(PreviewPane* previewPane);

public slots:
    void handleChangeSet(const ChangeSet& changeSet);

private:
    PreviewPane* previewPane_ = nullptr;
};
}  // namespace webide
