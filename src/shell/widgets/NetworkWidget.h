#pragma once

#include <QWidget>
#include <QVector>

class QLineEdit;
class QTableWidget;
class QTextEdit;

namespace webide {
struct NetworkRequest {
    QString id;
    QString url;
    QString method;
    QString status;
    QString size;
    QString timing;
    QString headers;
    QString payload;
};

class NetworkWidget : public QWidget {
    Q_OBJECT

public:
    explicit NetworkWidget(QWidget* parent = nullptr);
    void addRequest(const QString& url, const QString& method, const QString& status, const QString& size);
    void addRequest(const NetworkRequest& request);
    void clearRequests();

private:
    void rebuildTable();
    void updateDetails(int row);

    QLineEdit* filterInput_;
    QTableWidget* table_;
    QTextEdit* details_;
    QVector<NetworkRequest> requests_;
    QVector<int> visibleRows_;
};
}  // namespace webide
