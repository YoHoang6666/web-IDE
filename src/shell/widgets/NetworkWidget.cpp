#include "NetworkWidget.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLineEdit>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

namespace webide {
NetworkWidget::NetworkWidget(QWidget* parent)
    : QWidget(parent),
      filterInput_(new QLineEdit(this)),
      table_(new QTableWidget(this)),
      details_(new QTextEdit(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    filterInput_->setPlaceholderText(tr("Filter requests..."));

    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({tr("URL"), tr("Method"), tr("Status"), tr("Size"), tr("Timing")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    details_->setReadOnly(true);

    auto* splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(table_);
    splitter->addWidget(details_);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    layout->addWidget(filterInput_);
    layout->addWidget(splitter);

    connect(filterInput_, &QLineEdit::textChanged, this, [this]() { rebuildTable(); });
    connect(table_, &QTableWidget::currentCellChanged, this, [this](int row) { updateDetails(row); });
}

void NetworkWidget::addRequest(const QString& url, const QString& method, const QString& status, const QString& size) {
    NetworkRequest request;
    request.id = QString::number(requests_.size() + 1);
    request.url = url;
    request.method = method;
    request.status = status;
    request.size = size;
    request.timing = QStringLiteral("-");
    addRequest(request);
}

void NetworkWidget::addRequest(const NetworkRequest& request) {
    requests_.append(request);
    rebuildTable();
}

void NetworkWidget::clearRequests() {
    requests_.clear();
    table_->setRowCount(0);
    details_->clear();
}

void NetworkWidget::rebuildTable() {
    table_->setRowCount(0);
    visibleRows_.clear();
    details_->clear();

    const QString filter = filterInput_->text().trimmed();
    for (int i = 0; i < requests_.size(); ++i) {
        const auto& request = requests_.at(i);
        if (!filter.isEmpty()) {
            const QString haystack = QStringLiteral("%1 %2 %3").arg(request.url, request.method, request.status);
            if (!haystack.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }
        }
        const int row = table_->rowCount();
        table_->insertRow(row);
        table_->setItem(row, 0, new QTableWidgetItem(request.url));
        table_->setItem(row, 1, new QTableWidgetItem(request.method));
        table_->setItem(row, 2, new QTableWidgetItem(request.status));
        table_->setItem(row, 3, new QTableWidgetItem(request.size));
        table_->setItem(row, 4, new QTableWidgetItem(request.timing));
        visibleRows_.append(i);
    }
}

void NetworkWidget::updateDetails(int row) {
    if (row < 0 || row >= visibleRows_.size()) {
        details_->clear();
        return;
    }
    const auto& request = requests_.at(visibleRows_.at(row));
    details_->setPlainText(tr("URL: %1\nMethod: %2\nStatus: %3\nSize: %4\nTiming: %5\n\nHeaders:\n%6\n\nPayload:\n%7")
                               .arg(request.url,
                                    request.method,
                                    request.status,
                                    request.size,
                                    request.timing,
                                    request.headers.isEmpty() ? tr("(none)") : request.headers,
                                    request.payload.isEmpty() ? tr("(none)") : request.payload));
}
}  // namespace webide
