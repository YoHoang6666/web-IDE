#include "TableModelAdapter.h"

namespace webide {
TableModelAdapter::TableModelAdapter(QObject* parent) : QAbstractTableModel(parent) {}

void TableModelAdapter::setRows(const QVariantList& rows) {
    beginResetModel();
    rows_ = rows;
    endResetModel();
}

int TableModelAdapter::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return rows_.size();
}

int TableModelAdapter::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return rows_.isEmpty() ? 0 : rows_.first().toMap().size();
}

QVariant TableModelAdapter::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole || index.row() >= rows_.size()) {
        return {};
    }

    const auto map = rows_.at(index.row()).toMap();
    return map.values().value(index.column());
}
}  // namespace webide
