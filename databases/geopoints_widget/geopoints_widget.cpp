#include "geopoints_widget.h"

#include <QDebug>
#include <QHeaderView>
#include <QTableView>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>

#include "ui_geopoints_widget.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    createConnection();
    QSqlTableModel *model = new QSqlTableModel();
    model->setTable("geo_points");
    model->select();
    ui->tableView_geopoints->setModel(model);
}

Widget::~Widget() { delete ui; }

bool Widget::createConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QApplication::applicationDirPath() + "/" + "geopoints");

    if (!db.open()) {
        qDebug() << "Ошибка:" << db.lastError().text();
        return false;
    }
    return true;
}
