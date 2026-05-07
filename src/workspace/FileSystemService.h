#pragma once

#include <QObject>
#include <QString>

namespace webide {
class FileSystemService : public QObject {
    Q_OBJECT

public:
    explicit FileSystemService(QObject* parent = nullptr);

    bool writeFile(const QString& path, const QString& content);
    QString readFile(const QString& path) const;

signals:
    void fileWritten(const QString& path);
};
}  // namespace webide
