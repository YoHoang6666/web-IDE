#include "FileSystemService.h"

#include <QFile>
#include <QTextStream>

namespace webide {
FileSystemService::FileSystemService(QObject* parent) : QObject(parent) {}

bool FileSystemService::writeFile(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream << content;
    emit fileWritten(path);
    return true;
}

QString FileSystemService::readFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    return stream.readAll();
}
}  // namespace webide
