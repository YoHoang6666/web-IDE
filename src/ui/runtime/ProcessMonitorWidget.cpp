#include "ProcessMonitorWidget.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace webide {
ProcessMonitorWidget::ProcessMonitorWidget(QWidget* parent)
    : QWidget(parent), table_(new QTableWidget(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(table_);

    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({tr("ID"), tr("Runtime"), tr("Command"), tr("Port"), tr("State"), tr("Exit")});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ProcessMonitorWidget::setProcesses(const QList<ManagedProcess>& processes) {
    table_->setRowCount(0);
    for (const ManagedProcess& process : processes) {
        const int row = table_->rowCount();
        table_->insertRow(row);
        table_->setItem(row, 0, new QTableWidgetItem(process.id));
        table_->setItem(row, 1, new QTableWidgetItem(process.runtimeId));
        table_->setItem(row, 2, new QTableWidgetItem(process.commandLabel));
        table_->setItem(row, 3, new QTableWidgetItem(process.port == 0 ? QStringLiteral("-") : QString::number(process.port)));
        table_->setItem(row, 4, new QTableWidgetItem(QString::number(static_cast<int>(process.state))));
        table_->setItem(row, 5, new QTableWidgetItem(QString::number(process.exitCode)));
    }
}

QString ProcessMonitorWidget::selectedProcessId() const {
    const QModelIndex index = table_->currentIndex();
    if (!index.isValid()) {
        return {};
    }
    const QTableWidgetItem* item = table_->item(index.row(), 0);
    return item ? item->text() : QString();
}
}  // namespace webide

