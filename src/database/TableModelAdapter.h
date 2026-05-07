#pragma once

#include <QAbstractTableModel>
#include <QVariantList>

namespace webide {
class TableModelAdapter : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit TableModelAdapter(QObject* parent = nullptr);

    void setRows(const QVariantList& rows);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QVariantList rows_;
};
}  // namespace webide
