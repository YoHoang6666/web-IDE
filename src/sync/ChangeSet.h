#pragma once

#include <QDateTime>
#include <QString>
#include <QVariantMap>

namespace webide {
enum class ChangeOrigin {
    Editor,
    FileSystem,
    Database,
    Preview
};

enum class ChangeKind {
    Created,
    Modified,
    Deleted,
    ReloadRequested,
    QueryExecuted
};

struct ChangeSet {
    ChangeOrigin origin;
    ChangeKind kind;
    QString resourcePath;
    QVariantMap metadata;
    QDateTime observedAt;
};
}  // namespace webide
