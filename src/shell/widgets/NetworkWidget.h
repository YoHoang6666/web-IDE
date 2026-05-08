#pragma once

#include <QWidget>

class QTableWidget;

namespace webide {
class NetworkWidget : public QWidget {
    Q_OBJECT

public:
    explicit NetworkWidget(QWidget* parent = nullptr);
    void addRequest(const QString& url, const QString& method, const QString& status, const QString& size);
    void clearRequests();

private:
    QTableWidget* table_;
};
}  // namespace webide
