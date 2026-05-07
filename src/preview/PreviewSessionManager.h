#pragma once

#include <QObject>
#include <QHash>
#include <QUrl>

namespace webide {
class PreviewSessionManager : public QObject {
    Q_OBJECT

public:
    explicit PreviewSessionManager(QObject* parent = nullptr);
    void assignTarget(const QString& documentPath, const QUrl& url);
    QUrl targetFor(const QString& documentPath) const;

private:
    QHash<QString, QUrl> targets_;
};
}  // namespace webide
