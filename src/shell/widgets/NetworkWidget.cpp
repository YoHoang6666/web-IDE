#include "NetworkWidget.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace webide {
NetworkWidget::NetworkWidget(QWidget* parent) : QWidget(parent), table_(new QTableWidget(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(table_);

    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({tr("URL"), tr("Method"), tr("Status"), tr("Size")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void NetworkWidget::addRequest(const QString& url, const QString& method, const QString& status, const QString& size) {
    const int row = table_->rowCount();
    table_->insertRow(row);
    table_->setItem(row, 0, new QTableWidgetItem(url));
    table_->setItem(row, 1, new QTableWidgetItem(method));
    table_->setItem(row, 2, new QTableWidgetItem(status));
    table_->setItem(row, 3, new QTableWidgetItem(size));
}

void NetworkWidget::clearRequests() { table_->setRowCount(0); }
}  // namespace webide
