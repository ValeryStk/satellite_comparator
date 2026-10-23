#include "geopoints_widget.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableView>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>

#include "ui_geopoints_widget.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    QString pathToDb = QApplication::applicationDirPath() + "/" + "geopoints";
    initDataBase(pathToDb);
    QSqlTableModel *model = new QSqlTableModel();
    model->setTable("geo_points");
    model->select();
    ui->tableView_geopoints->setModel(model);
}

Widget::~Widget() { delete ui; }

bool Widget::initDataBase(const QString &dbPath) {
    // Важно проверить существование файла ДО вызова db.open(),
    // так как open() для SQLite автоматически создает пустой файл на диске.
    bool dbExists = QFileInfo::exists(dbPath);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QMessageBox::critical(
            nullptr, "Ошибка БД",
            "Не удалось открыть базу данных:\n" + db.lastError().text());
        return false;
    }

    // Если файла не существовало, создаем таблицу по вашей схеме
    if (!dbExists) {
        QSqlQuery query;
        QString createTableQuery =
            "CREATE TABLE geo_points ("
            "    point_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    product_uri TEXT    NOT NULL UNIQUE,"
            "    latitude    REAL    NOT NULL,"
            "    longitude   REAL    NOT NULL,"
            "    name        TEXT    NOT NULL,"
            "    description TEXT"
            ");";

        if (!query.exec(createTableQuery)) {
            QMessageBox::critical(nullptr, "Ошибка создания таблицы",
                                  "Не удалось создать таблицу geo_points:\n" +
                                      query.lastError().text());
            return false;
        }
        qDebug() << "Создан новый файл БД и таблица geo_points по пути:"
                 << dbPath;
    } else {
        qDebug() << "Успешно подключена существующая БД по пути:" << dbPath;
    }

    return true;
}
