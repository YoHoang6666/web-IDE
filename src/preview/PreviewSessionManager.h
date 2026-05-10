#pragma once

#include <QObject>
#include <QHash>
#include <QUrl>

namespace webide {
enum class PreviewServerStatus { Unknown, Starting, Running, Unreachable, Stopped };

struct PreviewSession {
    QString documentPath;
    QUrl targetUrl;
    QUrl webSocketUrl;
    bool hotReloadEnabled = false;
    bool trustLocalhost = false;
    PreviewServerStatus status = PreviewServerStatus::Unknown;
    QString statusDetail;
};

class PreviewSessionManager : public QObject {
    Q_OBJECT

public:
    explicit PreviewSessionManager(QObject* parent = nullptr);
    void assignTarget(const QString& documentPath, const QUrl& url);
    void assignSession(const PreviewSession& session);
    PreviewSession sessionFor(const QString& documentPath) const;
    void updateStatus(const QString& documentPath, PreviewServerStatus status, const QString& detail = QString());

private:
    QHash<QString, PreviewSession> sessions_;
};
}  // namespace webide
