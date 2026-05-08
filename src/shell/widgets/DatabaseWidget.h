#pragma once

#include <QWidget>

class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QSqlDatabase;
class QSqlQueryModel;
class QSqlTableModel;
class QTableView;

namespace webide {
class DatabaseWidget : public QWidget {
    Q_OBJECT

public:
    explicit DatabaseWidget(QWidget* parent = nullptr);
    ~DatabaseWidget() override;

    bool openDatabase(const QString& path);
    QString currentDatabasePath() const;

signals:
    void statusMessage(const QString& message);
    void databaseOpened(const QString& path);

private:
    void setupUi();
    void loadTables();
    void showTable(const QString& tableName);
    void runQuery();
    void insertRow();
    void deleteSelectedRows();

    QListWidget* tablesList_;
    QTableView* tableView_;
    QTableView* queryResultsView_;
    QPlainTextEdit* sqlEditor_;
    QPushButton* runQueryButton_;
    QPushButton* insertRowButton_;
    QPushButton* deleteRowButton_;

    QSqlTableModel* tableModel_;
    QSqlQueryModel* queryModel_;

    QString connectionName_;
    QString databasePath_;
};
}  // namespace webide
