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
    model->setTable("geo_points");
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
    QSqlQuery query;

    // Подготавливаем безопасный SQL-запрос с плейсхолдерами
    query.prepare(
        "INSERT INTO geo_points (product_uri, latitude, longitude, name, "
        "description) "
        "VALUES (:product_uri, :latitude, :longitude, :name, :description);");

    // Привязываем значения к плейсхолдерам
    query.bindValue(":product_uri", productUri);
    query.bindValue(":latitude", latitude);
    query.bindValue(":longitude", longitude);
    query.bindValue(":name", name);

    // Если описание пустое, отправляем в базу данных NULL, иначе саму строку
    if (description.isEmpty()) {
        query.bindValue(":description", QVariant(QVariant::String));
    } else {
        query.bindValue(":description", description);
    }

    // Выполняем запрос
    if (!query.exec()) {
        qWarning() << "Ошибка вставки в таблицу geo_points:"
                   << query.lastError().text();
        return false;
    }

    return true;
}

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

void Widget::generateAndInsertRandomPoints(int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> latDist(-90.0, 90.0);
    std::uniform_real_distribution<double> lonDist(-180.0, 180.0);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();  // Включаем транзакцию для ускорения записи

    QSqlQuery query;
    query.prepare(
        "INSERT INTO geo_points (product_uri, latitude, longitude, name, "
        "description) "
        "VALUES (:uri, :lat, :lon, :name, :desc);");

    for (int i = 0; i < count; ++i) {
        QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);

        query.bindValue(":uri", QString("https://example.com_%1").arg(uid));
        query.bindValue(":lat", latDist(gen));
        query.bindValue(":lon", lonDist(gen));
        query.bindValue(":name", QString("Точка-%1").arg(uid.left(8)));
        query.bindValue(":desc", QString("Автогенерация теста"));

        if (!query.exec()) {
            qWarning() << "Ошибка вставки во время генерации:"
                       << query.lastError().text();
        }
    }

    db.commit();  // Сохраняем все точки на диск одним пакетом
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
