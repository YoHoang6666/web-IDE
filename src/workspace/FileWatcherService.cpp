#include "FileWatcherService.h"

#include <QTimer>

namespace webide {
FileWatcherService::FileWatcherService(QObject* parent)
    : QObject(parent), debounceTimer_(new QTimer(this)) {
    debounceTimer_->setSingleShot(true);
    debounceTimer_->setInterval(120);
    connect(debounceTimer_, &QTimer::timeout, this, [this]() {
        emit changeDetected(ChangeSet{ChangeOrigin::FileSystem, ChangeKind::Modified, pendingPath_, {}, QDateTime::currentDateTimeUtc()});
    });
}

void FileWatcherService::enqueueExternalChange(const QString& path) {
    pendingPath_ = path;
    debounceTimer_->start();
}
}  // namespace webide
