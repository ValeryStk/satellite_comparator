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
QSqlQueryModel *model;

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);

    QString pathToDb = QApplication::applicationDirPath() + "/" + "geopoints";
    if (!initDataBase(pathToDb)) {
        return;
    }

    // 1. Создаем модель для JOIN-запросов
    model = new QSqlQueryModel(this);
    ui->tableView_geopoints->setModel(model);

    // 2. Динамически получаем самый первый URI из базы данных
    QString targetUri;
    QSqlQuery query("SELECT product_uri FROM uris ORDER BY id ASC LIMIT 1;");

    if (query.next()) {
        targetUri = query.value(0).toString();
        qDebug() << "Автоматически выбран первый найденный URI:" << targetUri;
    } else {
        qDebug() << "База данных пуста, URI не найдены.";
    }

    // Сохраняем текущий URI в переменную класса (если она у вас объявлена,
    // например m_currentUri) m_currentUri = targetUri;

    // 3. Вызываем метод фильтрации (если targetUri пустой, таблица просто
    // останется пустой)
    updatePointsTableByUri(targetUri);

    // Настройка контекстного меню
    ui->tableView_geopoints->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView_geopoints, &QTableView::customContextMenuRequested,
            this, &Widget::showContextMenu);

    // 3. Заполняем комбобокс актуальными URI из базы данных
    updateUriComboBox();

    // 4. Связываем переключение комбобокса с обновлением таблицы
    connect(ui->comboBox_uris, &QComboBox::currentTextChanged, this,
            [this](const QString &text) {
                m_currentUri = text;
                updatePointsTableByUri(m_currentUri);
            });

    // 5. Инициализируем начальное состояние: берем первый элемент из комбобокса
    if (ui->comboBox_uris->count() > 0) {
        m_currentUri = ui->comboBox_uris->currentText();
        updatePointsTableByUri(m_currentUri);
    } else {
        qDebug() << "База данных пуста, комбобокс URI не заполнен.";
    }
}

Widget::~Widget() { delete ui; }

bool Widget::insertGeoPoint(const QString &productUri, double latitude,
                            double longitude, const QString &name,
                            const QString &description) {
    Q_UNUSED(description);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();  // Заворачиваем в транзакцию для безопасности связей

    QSqlQuery query;

    // Шаг 1: Генерируем новый point_id (так как в новой схеме points нет
    // AUTOINCREMENT)
    int newPointId = 1;
    if (query.exec("SELECT MAX(point_id) FROM points;") && query.next()) {
        if (!query.value(0).isNull()) {
            newPointId = query.value(0).toInt() + 1;
        }
    }

    // Шаг 2: Вставляем саму географическую точку в таблицу points
    query.prepare(
        "INSERT INTO points (point_id, point_name, latitude, longitude) "
        "VALUES (:point_id, :point_name, :latitude, :longitude);");

    query.bindValue(":point_id", newPointId);

    // Если имя пустое, передаем NULL, чтобы избежать конфликта UNIQUE для
    // пустых строк
    if (name.isEmpty()) {
        query.bindValue(":point_name", QVariant(QVariant::String));
    } else {
        query.bindValue(":point_name", name);
    }

    query.bindValue(":latitude", latitude);
    query.bindValue(":longitude", longitude);

    if (!query.exec()) {
        qWarning() << "Ошибка вставки в таблицу points:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // Шаг 3: Добавляем URI в таблицу uris, если его там еще нет
    query.prepare(
        "INSERT OR IGNORE INTO uris (product_uri) VALUES (:product_uri);");
    query.bindValue(":product_uri", productUri);
    if (!query.exec()) {
        qWarning() << "Ошибка вставки в таблицу uris:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // Получаем id добавленного или уже существующего URI
    QVariant uriId = query.lastInsertId();
    if (!uriId.isValid() || uriId.toLongLong() == 0) {
        QSqlQuery selectQuery;
        selectQuery.prepare(
            "SELECT id FROM uris WHERE product_uri = :product_uri;");
        selectQuery.bindValue(":product_uri", productUri);
        if (selectQuery.exec() && selectQuery.next()) {
            uriId = selectQuery.value(0);
        } else {
            qWarning() << "Не удалось получить id для URI:" << productUri;
            db.rollback();
            return false;
        }
    }

    // Шаг 4: Связываем точку и URI в таблице pointId_uri
    query.prepare(
        "INSERT OR IGNORE INTO pointId_uri (point_id, uri_id) VALUES "
        "(:point_id, :uri_id);");
    query.bindValue(":point_id", newPointId);
    query.bindValue(":uri_id", uriId);

    if (!query.exec()) {
        qWarning() << "Ошибка вставки в таблицу pointId_uri:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

bool Widget::linkPointToUri(int pointId, const QString &productUri) {
    if (productUri.isEmpty()) {
        qWarning() << "Передан пустой URI.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();  // Заворачиваем в транзакцию для безопасности

    QSqlQuery query;

    // Шаг 1: Проверяем, существует ли вообще такая точка
    query.prepare("SELECT 1 FROM points WHERE point_id = :point_id;");
    query.bindValue(":point_id", pointId);
    if (!query.exec() || !query.next()) {
        qWarning() << "Точка с ID" << pointId << "не существует в базе данных.";
        db.rollback();
        return false;
    }

    // Шаг 2: Вставляем URI в таблицу uris (если его еще нет)
    query.prepare(
        "INSERT OR IGNORE INTO uris (product_uri) VALUES (:product_uri);");
    query.bindValue(":product_uri", productUri);
    if (!query.exec()) {
        qWarning() << "Ошибка вставки URI в таблицу uris:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // Шаг 3: Получаем ID этого URI
    QVariant uriId = query.lastInsertId();
    if (!uriId.isValid() || uriId.toLongLong() == 0) {
        // Если сработал IGNORE, находим существующий ID через SELECT
        QSqlQuery selectQuery;
        selectQuery.prepare(
            "SELECT id FROM uris WHERE product_uri = :product_uri;");
        selectQuery.bindValue(":product_uri", productUri);
        if (selectQuery.exec() && selectQuery.next()) {
            uriId = selectQuery.value(0);
        } else {
            qWarning() << "Не удалось получить ID для URI:" << productUri;
            db.rollback();
            return false;
        }
    }

    // Шаг 4: Связываем существующую точку с полученным uri_id
    // Использование OR IGNORE гарантирует отсутствие ошибок, если связь уже
    // была создана ранее
    query.prepare(
        "INSERT OR IGNORE INTO pointId_uri (point_id, uri_id) VALUES "
        "(:point_id, :uri_id);");
    query.bindValue(":point_id", pointId);
    query.bindValue(":uri_id", uriId);

    if (!query.exec()) {
        qWarning() << "Ошибка при создании связи в pointId_uri:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    qDebug() << "Точка" << pointId << "успешно связана с URI:" << productUri;
    return true;
}

QVector<PointData> Widget::Widget::selectPointsByProduct(
    const QString &productUri) {
    QVector<PointData> resultList;
    QSqlQuery query;

    // Используем JOIN для объединения таблиц по product_uri
    query.prepare(
        "SELECT p.point_id, p.point_name, p.product_uri, p.latitude, "
        "p.longitude "
        "FROM points p "
        "LEFT JOIN products pr ON p.product_uri = pr.product_uri "
        "WHERE pr.product_uri = :product_uri;");

    query.bindValue(":product_uri", productUri);

    if (!query.exec()) {
        qWarning() << "Ошибка выборки точек:" << query.lastError().text();
        return resultList;
    }

    // Читаем строки из полученного ответа базы данных
    while (query.next()) {
        PointData pt;
        pt.pointId = query.value("point_id").toInt();
        pt.pointName = query.value("point_name").toString();
        pt.productUri = query.value("product_uri").toString();
        pt.latitude = query.value("latitude").toDouble();
        pt.longitude = query.value("longitude").toDouble();

        resultList.append(pt);
    }

    return resultList;
}

bool Widget::initDataBase(const QString &dbPath) {
    bool dbExists = QFileInfo::exists(dbPath);

    // Предотвращаем дублирование соединений в Qt
    QSqlDatabase db;
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
    }
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

    // Если файла не существовало, создаем таблицы по новой схеме
    if (!dbExists) {
        QSqlQuery query;

        // Таблица 1: Географические точки
        QString createPointsTable =
            "CREATE TABLE points ("
            "    point_id   INTEGER PRIMARY KEY,"
            "    point_name TEXT UNIQUE,"
            "    latitude   REAL NOT NULL,"
            "    longitude  REAL"
            ");";

        if (!query.exec(createPointsTable)) {
            QMessageBox::critical(nullptr, "Ошибка создания таблицы points",
                                  "Не удалось создать таблицу points:\n" +
                                      query.lastError().text());
            return false;
        }

        // Таблица 2: Справочник URI продуктов
        QString createUrisTable =
            "CREATE TABLE uris ("
            "    id          INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    product_uri TEXT NOT NULL UNIQUE"
            ");";

        if (!query.exec(createUrisTable)) {
            QMessageBox::critical(nullptr, "Ошибка создания таблицы uris",
                                  "Не удалось создать таблицу uris:\n" +
                                      query.lastError().text());
            return false;
        }

        // Таблица 3: Связующая таблица Многие-ко-Многим (WITHOUT ROWID)
        // Добавлен составной PRIMARY KEY, необходимый для WITHOUT ROWID
        QString createPointIdUriTable =
            "CREATE TABLE pointId_uri ("
            "    point_id INTEGER REFERENCES points (point_id) ON DELETE "
            "CASCADE ON UPDATE CASCADE,"
            "    uri_id   INTEGER REFERENCES uris (id) ON DELETE CASCADE ON "
            "UPDATE CASCADE,"
            "    PRIMARY KEY (point_id, uri_id)"
            ") WITHOUT ROWID;";

        if (!query.exec(createPointIdUriTable)) {
            QMessageBox::critical(nullptr,
                                  "Ошибка создания таблицы pointId_uri",
                                  "Не удалось создать таблицу pointId_uri:\n" +
                                      query.lastError().text());
            return false;
        }

        // Индексы для внешних ключей, чтобы ускорить операции JOIN и каскадные
        // удаления
        query.exec(
            "CREATE INDEX idx_pointId_uri_point ON pointId_uri(point_id);");
        query.exec("CREATE INDEX idx_pointId_uri_uri ON pointId_uri(uri_id);");

        qDebug()
            << "Создан новый файл БД и таблицы по схеме Многие-ко-Многим в:"
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

    // Получаем текущее максимальное значение point_id, чтобы не было конфликтов
    // при повторном запуске генератора
    int startPointId = 1;
    QSqlQuery maxIdQuery("SELECT MAX(point_id) FROM points;");
    if (maxIdQuery.next() && !maxIdQuery.value(0).isNull()) {
        startPointId = maxIdQuery.value(0).toInt() + 1;
    }

    db.transaction();  // Включаем транзакцию для ускорения записи пакета данных

    QSqlQuery queryPoint;
    QSqlQuery queryUri;
    QSqlQuery queryLink;

    // Подготавливаем все три запроса заранее под новую структуру
    queryPoint.prepare(
        "INSERT INTO points (point_id, point_name, latitude, longitude) "
        "VALUES (:point_id, :name, :lat, :lon);");

    queryUri.prepare("INSERT OR IGNORE INTO uris (product_uri) VALUES (:uri);");

    queryLink.prepare(
        "INSERT OR IGNORE INTO pointId_uri (point_id, uri_id) VALUES "
        "(:point_id, :uri_id);");

    for (int i = 0; i < count; ++i) {
        QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString currentUri = QString("https://example.com_%1").arg(uid);
        QString currentName = QString("Точка-%1").arg(uid.left(8));
        int currentPointId = startPointId + i;

        // 1. Вставляем саму географическую точку
        queryPoint.bindValue(":point_id", currentPointId);
        queryPoint.bindValue(":name", currentName);
        queryPoint.bindValue(":lat", latDist(gen));
        queryPoint.bindValue(":lon", lonDist(gen));

        if (!queryPoint.exec()) {
            qWarning() << "Ошибка генерации (points):"
                       << queryPoint.lastError().text();
            continue;  // Пропускаем шаг, если точка не создалась
        }

        // 2. Вставляем URI продукта
        queryUri.bindValue(":uri", currentUri);
        if (!queryUri.exec()) {
            qWarning() << "Ошибка генерации (uris):"
                       << queryUri.lastError().text();
            continue;
        }

        // Получаем id только что вставленного (или уже существовавшего) URI
        QVariant uriId = queryUri.lastInsertId();

        // На случай, если сработал OR IGNORE и lastInsertId() пустой, находим
        // существующий id
        if (!uriId.isValid() || uriId.toLongLong() == 0) {
            QSqlQuery selectUri;
            selectUri.prepare("SELECT id FROM uris WHERE product_uri = :uri;");
            selectUri.bindValue(":uri", currentUri);
            if (selectUri.exec() && selectUri.next()) {
                uriId = selectUri.value(0);
            }
        }

        // 3. Создаем связь Многие-ко-Многим в таблице pointId_uri
        queryLink.bindValue(":point_id", currentPointId);
        queryLink.bindValue(":uri_id", uriId);

        if (!queryLink.exec()) {
            qWarning() << "Ошибка генерации (pointId_uri):"
                       << queryLink.lastError().text();
        }
    }

    db.commit();  // Сохраняем все изменения на диск единым пакетом
}

bool Widget::clearAllData() {
    QSqlDatabase db = QSqlDatabase::database();

    // Включаем транзакцию, чтобы очистка прошла атомарно и быстро
    db.transaction();

    QSqlQuery query;

    // 1. Удаляем все записи из таблицы точек
    // Каскадное удаление (ON DELETE CASCADE) автоматически очистит связанные
    // строки в pointId_uri
    if (!query.exec("DELETE FROM points;")) {
        qWarning() << "Ошибка очистки таблицы points:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // 2. Удаляем все записи из таблицы URI
    // Каскадное удаление также зачистит остатки в pointId_uri, если они были
    if (!query.exec("DELETE FROM uris;")) {
        qWarning() << "Ошибка очистки таблицы uris:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // 3. Сбрасываем внутренний счетчик AUTOINCREMENT для таблицы uris
    if (query.exec("SELECT 1 FROM sqlite_sequence WHERE name = 'uris';")) {
        query.exec("DELETE FROM sqlite_sequence WHERE name = 'uris';");
    }

    db.commit();
    qDebug() << "Все три таблицы успешно очищены, счетчик ID для uris сброшен.";
    return true;
}

void Widget::updateUriComboBox() {
    // Временно блокируем сигналы, чтобы при заполнении комбобокса не вызывалось
    // автоматическое обновление таблицы
    ui->comboBox_uris->blockSignals(true);
    ui->comboBox_uris->clear();

    QSqlQuery query("SELECT product_uri FROM uris ORDER BY product_uri ASC;");
    while (query.next()) {
        ui->comboBox_uris->addItem(query.value(0).toString());
    }

    ui->comboBox_uris->blockSignals(false);
}

void Widget::updatePointsTableByUri(const QString &productUri) {
    QSqlQuery query;
    // Пишем JOIN запрос для связи трех таблиц и фильтрации по текстовому URI
    query.prepare(
        "SELECT p.point_id AS 'ID Точки', "
        "       p.point_name AS 'Название точки', "
        "       p.latitude AS 'Широта', "
        "       p.longitude AS 'Долгота' "
        "FROM points p "
        "JOIN pointId_uri link ON p.point_id = link.point_id "
        "JOIN uris u ON link.uri_id = u.id "
        "WHERE u.product_uri = :product_uri;");

    query.bindValue(":product_uri", productUri);

    if (query.exec()) {
        model->setQuery(
            std::move(query));  // Обновляем данные в модели отображения
        ui->tableView_geopoints->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
    } else {
        qWarning() << "Не удалось отфильтровать точки по URI:"
                   << query.lastError().text();
    }
}

void Widget::showAllPointsWithUris() {
    QSqlQuery query;

    // Запрос выбирает все точки, объединяет их с URI через таблицу связей,
    // группирует по ID точки и склеивает все её URI в одну строку через
    // запятую. Слева (LEFT JOIN) используются джоины, чтобы показались даже те
    // точки, у которых временно нет URI.
    query.prepare(
        "SELECT p.point_id AS 'ID Точки', "
        "       p.point_name AS 'Название точки', "
        "       p.latitude AS 'Широта', "
        "       p.longitude AS 'Долгота', "
        "       group_concat(u.product_uri, ', ') AS 'Связанные URI' "
        "FROM points p "
        "LEFT JOIN pointId_uri link ON p.point_id = link.point_id "
        "LEFT JOIN uris u ON link.uri_id = u.id "
        "GROUP BY p.point_id;");

    if (query.exec()) {
        model->setQuery(std::move(query));  // Обновляем модель на экране
        ui->tableView_geopoints->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        qDebug() << "Таблица переведена в режим отображения всех точек.";
    } else {
        qWarning() << "Не удалось загрузить все точки:"
                   << query.lastError().text();
    }
}

void Widget::on_pushButton_clicked() {
    generateAndInsertRandomPoints(1);
    updateUriComboBox();
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
        // 1. Получаем point_id из первого столбца выделенной строки (индекс
        // столбца 0)
        int row = index.row();
        int pointId =
            ui->tableView_geopoints->model()->index(row, 0).data().toInt();

        // 2. Выполняем явный SQL-запрос на удаление
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM points WHERE point_id = :point_id;");
        deleteQuery.bindValue(":point_id", pointId);

        if (deleteQuery.exec()) {
            qDebug() << "Точка с ID" << pointId << "успешно удалена из БД.";

            // 3. Синхронизируем состояние View — принудительно обновляем
            // таблицу. Здесь вызывайте тот метод обновления, в режиме которого
            // сейчас находится приложение. Например, вывод ВСЕХ точек:
            showAllPointsWithUris();

            // Или, если вы отображаете точки для конкретного URI,
            // раскомментируйте строку ниже:
            // updatePointsTableByUri(m_currentUri);
        } else {
            qWarning() << "Не удалось удалить точку из БД:"
                       << deleteQuery.lastError().text();
            QMessageBox::warning(this, "Ошибка удаления",
                                 "Не удалось удалить точку из базы данных.");
        }

        updateUriComboBox();
    }
}

void Widget::on_pushButton_insert_data_clicked() {
    // Получаем строки из UI-элементов формы (замените имена указателей на ваши)
    QString uri = ui->lineEditUri->text().trimmed();
    QString name = ui->lineEditName->text().trimmed();

    // Переводим текст координат в double локально-независимым способом
    bool latOk, lonOk;
    double lat = ui->lineEditLatitude->text().toDouble(&latOk);
    double lon = ui->lineEditLongitude->text().toDouble(&lonOk);

    // Валидация входных данных перед отправкой в БД
    if (uri.isEmpty()) {
        QMessageBox::warning(this, "Валидация",
                             "Поле Product URI не может быть пустым!");
        return;
    }
    if (!latOk) {
        QMessageBox::warning(this, "Валидация",
                             "Широта (Latitude) должна быть числом!");
        return;
    }

    // Вызываем ранее созданную функцию вставки (она сама запишет и в products,
    // и в points) Передаем 0 вместо долготы или QVariant(), если lonOk == false
    // (так как в БД это поле NULL)
    double longitudeValue = lonOk ? lon : 0.0;

    if (insertGeoPoint(uri, lat, longitudeValue, name, "")) {
        QMessageBox::information(this, "Успех",
                                 "Точка успешно сохранена в базу данных!");

        // Очищаем поля формы для следующего ввода (опционально)
        ui->lineEditName->clear();
        ui->lineEditLatitude->clear();
        ui->lineEditLongitude->clear();
    } else {
        QMessageBox::critical(this, "Ошибка",
                              "Не удалось добавить точку. Проверьте "
                              "уникальность имени (UNIQUE)!");
    }
    // model->select();
}

void Widget::on_pushButton_clear_all_data_clicked() {
    clearAllData();
    // model->select();
}

void Widget::on_pushButton_show_all_points_clicked() {
    showAllPointsWithUris();
}

void Widget::on_comboBox_uris_currentIndexChanged(const QString &arg1) {}
