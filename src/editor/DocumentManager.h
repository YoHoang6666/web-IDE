#pragma once

#include <QObject>
#include <QHash>
#include <QString>

namespace webide {
class FileSystemService;

class DocumentManager : public QObject {
    Q_OBJECT

public:
    explicit DocumentManager(QObject* parent = nullptr);

    void bindFileSystem(FileSystemService* fileSystemService);
    void updateBuffer(const QString& path, const QString& content);
    bool saveDocument(const QString& path);
    QString contentFor(const QString& path) const;

signals:
    void documentSaved(const QString& path, const QString& content);

private:
    FileSystemService* fileSystemService_ = nullptr;
    QHash<QString, QString> buffers_;
};
}  // namespace webide
