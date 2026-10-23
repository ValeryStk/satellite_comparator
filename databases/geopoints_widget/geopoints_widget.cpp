#include "geopoints_widget.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QTableView>
#include <QTimer>
#include <QUuid>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>
#include <random>

#include "ui_geopoints_widget.h"
QSqlTableModel *model;

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    QString pathToDb = QApplication::applicationDirPath() + "/" + "geopoints";
    initDataBase(pathToDb);
    model = new QSqlTableModel();
    model->setTable("points");
    model->select();
    // Вставьте этот код в конструктор вашего виджета (например,
    // GeoPointsWidget)
    ui->tableView_geopoints->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->tableView_geopoints, &QTableView::customContextMenuRequested,
            this, &Widget::showContextMenu);

    ui->tableView_geopoints->setModel(model);
}

Widget::~Widget() { delete ui; }

bool Widget::insertGeoPoint(const QString &productUri, double latitude,
                            double longitude, const QString &name,
                            const QString &description) {
    // Параметр description из старого метода опущен, так как в вашей новой
    // таблице points его нет. Поле longitude теперь может принимать NULL,
    // проверяем его.
    Q_UNUSED(description);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();  // Заворачиваем в транзакцию для безопасности связей

    QSqlQuery query;

    // Шаг 1: Добавляем URI в таблицу products, если его там еще нет
    query.prepare(
        "INSERT OR IGNORE INTO products (product_uri) VALUES (:product_uri);");
    query.bindValue(":product_uri", productUri);
    if (!query.exec()) {
        qWarning() << "Ошибка вставки в таблицу products:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // Шаг 2: Вставляем саму географическую точку в таблицу points
    query.prepare(
        "INSERT INTO points (point_name, product_uri, latitude, longitude) "
        "VALUES (:point_name, :product_uri, :latitude, :longitude);");

    // Если имя пустое, передаем NULL (так как у вас наложено ограничение
    // UNIQUE, пустые строки вызовут конфликт)
    if (name.isEmpty()) {
        query.bindValue(":point_name", QVariant(QVariant::String));
    } else {
        query.bindValue(":point_name", name);
    }

    query.bindValue(":product_uri", productUri);
    query.bindValue(":latitude", latitude);
    query.bindValue(":longitude", longitude);

    if (!query.exec()) {
        qWarning() << "Ошибка вставки в таблицу points:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

bool Widget::initDataBase(const QString &dbPath) {
    bool dbExists = QFileInfo::exists(dbPath);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QMessageBox::critical(
            nullptr, "Ошибка БД",
            "Не удалось открыть базу данных:\n" + db.lastError().text());
        return false;
    }

    // Включаем поддержку Foreign Keys в SQLite (обязательно при каждом
    // подключении)
    QSqlQuery pragmaQuery;
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");

    // Если файла не существовало, создаем таблицы по вашей новой схеме
    if (!dbExists) {
        QSqlQuery query;

        // Таблица 1: Справочник продуктов (создается первой)
        QString createProductsTable =
            "CREATE TABLE products ("
            "    id          INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    product_uri TEXT    NOT NULL UNIQUE"
            ");";

        if (!query.exec(createProductsTable)) {
            QMessageBox::critical(nullptr, "Ошибка создания таблицы products",
                                  "Не удалось создать таблицу products:\n" +
                                      query.lastError().text());
            return false;
        }

        // Таблица 2: Географические точки (ссылается на products)
        QString createPointsTable =
            "CREATE TABLE points ("
            "    point_id    INTEGER PRIMARY KEY,"
            "    point_name  TEXT    UNIQUE,"  // SQLite не имеет типа данных
                                               // TEXT для point_name по
                                               // умолчанию, явно указан TEXT
            "    product_uri TEXT    REFERENCES products (product_uri),"
            "    latitude    REAL    NOT NULL,"
            "    longitude   REAL"
            ");";

        if (!query.exec(createPointsTable)) {
            QMessageBox::critical(nullptr, "Ошибка создания таблицы points",
                                  "Не удалось создать таблицу points:\n" +
                                      query.lastError().text());
            return false;
        }

        qDebug() << "Создан новый файл БД и таблицы по новой схеме в:"
                 << dbPath;
    } else {
        qDebug() << "Успешно подключена существующая БД по пути:" << dbPath;
    }

    return true;
}

void Widget::generateAndInsertRandomPoints(int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> latDist(-90.0, 90.0);
    std::uniform_real_distribution<double> lonDist(-180.0, 180.0);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();  // Включаем транзакцию для ускорения записи пакета данных

    QSqlQuery queryProd;
    QSqlQuery queryPoint;

    // Подготавливаем оба запроса заранее
    queryProd.prepare(
        "INSERT OR IGNORE INTO products (product_uri) VALUES (:uri);");
    queryPoint.prepare(
        "INSERT INTO points (point_name, product_uri, latitude, longitude) "
        "VALUES (:name, :uri, :lat, :lon);");

    for (int i = 0; i < count; ++i) {
        QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString currentUri = QString("https://example.com_%1").arg(uid);
        QString currentName = QString("Точка-%1").arg(uid.left(8));

        // 1. Вставка в родительскую таблицу products
        queryProd.bindValue(":uri", currentUri);
        if (!queryProd.exec()) {
            qWarning() << "Ошибка генерации (products):"
                       << queryProd.lastError().text();
            continue;  // Пропускаем итерацию, если что-то пошло не так
        }

        // 2. Вставка в дочернюю таблицу points
        queryPoint.bindValue(":name", currentName);
        queryPoint.bindValue(":uri", currentUri);
        queryPoint.bindValue(":lat", latDist(gen));
        queryPoint.bindValue(":lon", lonDist(gen));

        if (!queryPoint.exec()) {
            qWarning() << "Ошибка генерации (points):"
                       << queryPoint.lastError().text();
        }
    }

    db.commit();  // Сохраняем все изменения на диск одним пакетом
}

void Widget::on_pushButton_clicked() {
    generateAndInsertRandomPoints(5);
    model->select();
}

void Widget::showContextMenu(const QPoint &pos) {
    // Получаем индекс ячейки, по которой кликнули правой кнопкой мыши
    QModelIndex index = ui->tableView_geopoints->indexAt(pos);
    if (!index.isValid()) return;  // Кликнули по пустому месту таблицы

    // Создаем контекстное меню
    QMenu menu(this);
    QAction *deleteAction = menu.addAction("Удалить точку");

    // Показываем меню в текущей позиции курсора
    QAction *selectedAction =
        menu.exec(ui->tableView_geopoints->viewport()->mapToGlobal(pos));

    if (selectedAction == deleteAction) {
        // 1. Удаляем строку из модели (она сама отправит DELETE в SQLite)
        // Обратите внимание: у модели должна быть стратегия
        // QSqlTableModel::OnRowChange (по умолчанию)
        model->removeRow(index.row());

        // 2. Синхронизируем состояние, чтобы View перерисовал строки
        model->select();
    }
}
