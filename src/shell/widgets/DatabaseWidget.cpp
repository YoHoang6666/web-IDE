#include "DatabaseWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableView>
#include <QUuid>
#include <QVBoxLayout>

namespace webide {
DatabaseWidget::DatabaseWidget(QWidget* parent)
    : QWidget(parent),
      tablesList_(new QListWidget(this)),
      tableView_(new QTableView(this)),
      queryResultsView_(new QTableView(this)),
      sqlEditor_(new QPlainTextEdit(this)),
      runQueryButton_(new QPushButton(tr("Run Query"), this)),
      insertRowButton_(new QPushButton(tr("Insert Row"), this)),
      deleteRowButton_(new QPushButton(tr("Delete Row"), this)),
      tableModel_(new QSqlTableModel(this)),
      queryModel_(new QSqlQueryModel(this)),
      connectionName_(QStringLiteral("webide-db-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces))) {
    setupUi();

    tableView_->setModel(tableModel_);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setAlternatingRowColors(true);

    queryResultsView_->setModel(queryModel_);
    queryResultsView_->setAlternatingRowColors(true);

    connect(tablesList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            showTable(item->text());
        }
    });
    connect(runQueryButton_, &QPushButton::clicked, this, &DatabaseWidget::runQuery);
    connect(insertRowButton_, &QPushButton::clicked, this, &DatabaseWidget::insertRow);
    connect(deleteRowButton_, &QPushButton::clicked, this, &DatabaseWidget::deleteSelectedRows);
}

DatabaseWidget::~DatabaseWidget() {
    if (QSqlDatabase::contains(connectionName_)) {
        auto db = QSqlDatabase::database(connectionName_);
        db.close();
        QSqlDatabase::removeDatabase(connectionName_);
    }
}

void DatabaseWidget::setupUi() {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 8, 8, 8);

    auto* horizontalSplit = new QSplitter(Qt::Horizontal, this);
    horizontalSplit->addWidget(tablesList_);

    auto* rightPane = new QWidget(horizontalSplit);
    auto* rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto* topBar = new QHBoxLayout();
    topBar->addWidget(new QLabel(tr("Table Data"), rightPane));
    topBar->addStretch();
    topBar->addWidget(insertRowButton_);
    topBar->addWidget(deleteRowButton_);

    rightLayout->addLayout(topBar);
    rightLayout->addWidget(tableView_, 2);
    rightLayout->addWidget(new QLabel(tr("Query Results"), rightPane));
    rightLayout->addWidget(queryResultsView_, 1);
    rightLayout->addWidget(new QLabel(tr("SQL Query"), rightPane));
    rightLayout->addWidget(sqlEditor_, 1);
    rightLayout->addWidget(runQueryButton_);

    horizontalSplit->addWidget(rightPane);
    horizontalSplit->setStretchFactor(0, 1);
    horizontalSplit->setStretchFactor(1, 3);

    rootLayout->addWidget(horizontalSplit);
}

bool DatabaseWidget::openDatabase(const QString& path) {
    if (path.isEmpty()) {
        return false;
    }

    if (QSqlDatabase::contains(connectionName_)) {
        auto db = QSqlDatabase::database(connectionName_);
        db.close();
        QSqlDatabase::removeDatabase(connectionName_);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::warning(this, tr("Database"), tr("Failed to open database: %1").arg(db.lastError().text()));
        return false;
    }

    databasePath_ = path;
    tableModel_->setTable({});
    tableModel_->setEditStrategy(QSqlTableModel::OnFieldChange);
    queryModel_->setQuery(QSqlQuery(db));

    loadTables();
    emit statusMessage(tr("Opened database: %1").arg(path));
    emit databaseOpened(path);
    return true;
}

QString DatabaseWidget::currentDatabasePath() const { return databasePath_; }

void DatabaseWidget::loadTables() {
    tablesList_->clear();
    if (!QSqlDatabase::contains(connectionName_)) {
        return;
    }

    const auto db = QSqlDatabase::database(connectionName_);
    const QStringList tables = db.tables();
    tablesList_->addItems(tables);
    if (!tables.isEmpty()) {
        showTable(tables.first());
    }
}

void DatabaseWidget::showTable(const QString& tableName) {
    if (!QSqlDatabase::contains(connectionName_) || tableName.isEmpty()) {
        return;
    }

    tableModel_->setTable(tableName);
    tableModel_->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!tableModel_->select()) {
        emit statusMessage(tr("Failed to load table: %1").arg(tableModel_->lastError().text()));
        return;
    }

    tableView_->resizeColumnsToContents();
}

void DatabaseWidget::runQuery() {
    if (!QSqlDatabase::contains(connectionName_)) {
        return;
    }

    const QString sql = sqlEditor_->toPlainText().trimmed();
    if (sql.isEmpty()) {
        return;
    }

    const auto db = QSqlDatabase::database(connectionName_);
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        emit statusMessage(tr("Query failed: %1").arg(query.lastError().text()));
        return;
    }

    queryModel_->setQuery(query);
    emit statusMessage(tr("Query executed"));

    loadTables();
}

void DatabaseWidget::insertRow() {
    if (tableModel_->tableName().isEmpty()) {
        return;
    }

    const int row = tableModel_->rowCount();
    if (!tableModel_->insertRow(row)) {
        emit statusMessage(tr("Failed to insert row"));
        return;
    }

    tableView_->selectRow(row);
    emit statusMessage(tr("Inserted row"));
}

void DatabaseWidget::deleteSelectedRows() {
    const QModelIndex index = tableView_->currentIndex();
    if (!index.isValid()) {
        return;
    }

    if (!tableModel_->removeRow(index.row())) {
        emit statusMessage(tr("Failed to delete row"));
        return;
    }

    if (!tableModel_->submitAll()) {
        emit statusMessage(tr("Failed to apply row deletion: %1").arg(tableModel_->lastError().text()));
        tableModel_->revertAll();
        return;
    }

    tableModel_->select();
    emit statusMessage(tr("Deleted row"));
}
}  // namespace webide
