#include "AtmCorrectionMainWindow.h"

#include <QDebug>

#include "QStringList"
#include "double_delegate.cpp"
#include "satellites_structs.h"
#include "ui_AtmCorrectionMainWindow.h"

QVector<QColor> channelColors = {
    QColor("#e41a1c"), QColor("#377eb8"), QColor("#4daf4a"), QColor("#984ea3"),
    QColor("#ff7f00"), QColor("#ffff33"), QColor("#a65628"), QColor("#f781bf"),
    QColor("#999999"), QColor("#66c2a5")};

calculation_solver *cs;

AtmCorrectionMainWindow::AtmCorrectionMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AtmCorrectionMainWindow) {
    ui->setupUi(this);
    atm_params_plot = ui->widget_atm_params;
    QColor bg(45, 45, 45);  // тёмно-серый
    QColor axisBg(35, 35, 35);  // чуть темнее внутри области графика
    QColor lightGray(200, 200, 200);

    ui->widget_atm_params->xAxis->setTickLabelColor(lightGray);
    ui->widget_atm_params->yAxis->setTickLabelColor(lightGray);

    ui->widget_atm_params->xAxis->setLabelColor(lightGray);
    ui->widget_atm_params->yAxis->setLabelColor(lightGray);

    ui->widget_atm_params->setBackground(bg);
    ui->widget_atm_params->axisRect()->setBackground(axisBg);

    ui->comboBox_satellite_type->blockSignals(true);
    ui->comboBox_satellite_type->addItems({"sentinel 2A", "sentinel 2B"});
    ui->comboBox_satellite_type->blockSignals(false);
    QStringList sl;
    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i)
        sl << sad::sentinel_2A_gui_band_names[i];
    bands_widget = new BandsWidget(sl, ui->verticalLayout_checkBoxes);
    // bands_widget->show();

    ui->doubleSpinBox_sunZenitAngle->setValue(45);
    qRegisterMetaType<result_values>();
    cs = new calculation_solver({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    connect(cs, &calculation_solver::darkpixels_calculation_finished, this,
            &AtmCorrectionMainWindow::showResult);
    auto lambdas = cs->getLambdaList();
    auto responses = cs->getResponsesList();
    auto w = QVector<double>::fromStdVector(lambdas);

    const int graphCount =
        std::min<int>(responses.size(), channelColors.size());

    atm_params_plot->clearGraphs();

    for (int i = 0; i < graphCount; ++i) {
        atm_params_plot->addGraph();
        QPen pen(channelColors[i]);
        pen.setWidth(2);
        atm_params_plot->graph(i)->setPen(pen);
        atm_params_plot->graph(i)->setData(
            w, QVector<double>::fromStdVector(responses[i]));
    }
    QPen pen("#0ecfe1");
    pen.setWidth(3);
    atm_params_plot->addGraph();  // для Т_H20
    atm_params_plot->graph(10)->setPen(pen);

    /*atm_params_plot->addGraph();
    int next_index = responses.size() - 1;
    atm_params_plot->graph(next_index)->setPen(QPen("#1bcdcd"));
    auto h20 = QVector<double>::fromStdVector(cs->get_h2o());
    atm_params_plot->graph(responses.size() - 1)->setData(w, h20);
    atm_params_plot->addGraph();
    ++next_index;
    atm_params_plot->graph(next_index)->setPen(QPen("#cd1ba0"));
    auto o3 = QVector<double>::fromStdVector(cs->get_o3());
    atm_params_plot->graph(next_index)->setData(w, o3);*/
    atm_params_plot->rescaleAxes(true);
    atm_params_plot->replot();

    ui->tableWidget_uknown_params->setColumnCount(11);
    ui->tableWidget_uknown_params->setRowCount(3);
    ui->tableWidget_uknown_params->verticalHeader()->setVisible(false);
    ui->tableWidget_uknown_params->setHorizontalHeaderLabels(
        {"--", "X", "q", "p", "Tau_m0", "Tau_a0", "Beta", "Tau_e", "g_a", "p_1",
         "p_2"});
    for (int row = 0; row < ui->tableWidget_uknown_params->rowCount(); ++row) {
        // Если это вторая строка (индекс 1) — пропускаем её, она останется
        // доступной для изменения
        if (row == 0) {
            continue;
        }

        for (int col = 0; col < ui->tableWidget_uknown_params->columnCount();
             ++col) {
            QTableWidgetItem *item =
                ui->tableWidget_uknown_params->item(row, col);

            // Если ячейка еще не создана, создаем ее
            if (!item) {
                item = new QTableWidgetItem();
                ui->tableWidget_uknown_params->setItem(row, col, item);
            }

            // Убираем флаг редактирования для всех остальных строк
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setTextAlignment(Qt::AlignCenter);
        }
    }

    // Координаты первой ячейки второй строки: row = 1, col = 0
    int blockRow = 1;
    int blockCol = 0;

    QTableWidgetItem *firstItem =
        ui->tableWidget_uknown_params->item(blockRow, blockCol);

    // Создаем ячейку, если она еще не была инициализирована
    if (!firstItem) {
        firstItem = new QTableWidgetItem();
        ui->tableWidget_uknown_params->setItem(blockRow, blockCol, firstItem);
    }

    // Убираем флаг редактирования у этой конкретной ячейки
    firstItem->setFlags(firstItem->flags() & ~Qt::ItemIsEditable);
    // Назначаем валидатор для всей таблицы
    ui->tableWidget_uknown_params->setItemDelegate(new DoubleDelegate(this));

    // Создаем элемент с нужным текстом
    QTableWidgetItem *firstCellItem = new QTableWidgetItem("Старт");
    QTableWidgetItem *secondCellItem = new QTableWidgetItem("Диапазон");
    QTableWidgetItem *resultCellItem = new QTableWidgetItem("Результат");

    // Устанавливаем его в первую строку (0) и первый столбец (0)
    ui->tableWidget_uknown_params->setItem(0, 0, firstCellItem);
    ui->tableWidget_uknown_params->setItem(1, 0, secondCellItem);
    ui->tableWidget_uknown_params->setItem(2, 0, resultCellItem);

    // Так как первая строка у вас заблокирована, не забудьте отключить
    // редактирование и для этой ячейки:
    // firstCellItem->setFlags(firstCellItem->flags() & ~Qt::ItemIsEditable);
    // Автоматическое растяжение всех 11 столбцов по ширине таблицы
    ui->tableWidget_uknown_params->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);

    // Автоматическое растяжение всех 5 строк по высоте таблицы
    ui->tableWidget_uknown_params->verticalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    // Привязываем имена к координатам ячеек (строка, столбец)
    cellMap["X"] = QPoint(0, 1);
    cellMap["q"] = QPoint(0, 2);
    cellMap["p"] = QPoint(0, 3);
    cellMap["Tau_m0"] = QPoint(0, 4);
    cellMap["Tau_a0"] = QPoint(0, 5);
    cellMap["Beta"] = QPoint(0, 6);
    cellMap["Tau_e"] = QPoint(0, 7);
    cellMap["g_a"] = QPoint(0, 8);
    cellMap["p_1"] = QPoint(0, 9);
    cellMap["p_2"] = QPoint(0, 10);

    cellMap["rng_X"] = QPoint(1, 1);
    cellMap["rng_q"] = QPoint(1, 2);
    cellMap["rng_p"] = QPoint(1, 3);
    cellMap["rng_Tau_m0"] = QPoint(1, 4);
    cellMap["rng_Tau_a0"] = QPoint(1, 5);
    cellMap["rng_Beta"] = QPoint(1, 6);
    cellMap["rng_Tau_e"] = QPoint(1, 7);
    cellMap["rng_g_a"] = QPoint(1, 8);
    cellMap["rng_p_1"] = QPoint(1, 9);
    cellMap["rng_p_2"] = QPoint(1, 10);

    cellMap["result_X"] = QPoint(2, 1);
    cellMap["result_q"] = QPoint(2, 2);
    cellMap["result_p"] = QPoint(2, 3);
    cellMap["result_Tau_m0"] = QPoint(2, 4);
    cellMap["result_Tau_a0"] = QPoint(2, 5);
    cellMap["result_Beta"] = QPoint(2, 6);
    cellMap["result_Tau_e"] = QPoint(2, 7);
    cellMap["result_g_a"] = QPoint(2, 8);
    cellMap["result_p_1"] = QPoint(2, 9);
    cellMap["result_p_2"] = QPoint(2, 10);

    setCellValue("X", 300);
    setCellValue("q", 2);
    setCellValue("p", 1.25);
    setCellValue("Tau_m0", 0.098, 3);
    setCellValue("Tau_a0", 0.2);
    setCellValue("Beta", 2);
    setCellValue("Tau_e", 0.04);
    setCellValue("g_a", 0.6);
    setCellValue("p_1", 0.05);
    setCellValue("p_2", 0.15);

    setCellStringValue("rng_X", "280 - 350");
    setCellStringValue("rng_q", "1 - 6");
    setCellStringValue("rng_p", "0.5 - 2.0");
    setCellStringValue("rng_Tau_m0", "0.09 - 0.1");
    setCellStringValue("rng_Tau_a0", "0.01 - 1.5");
    setCellStringValue("rng_Beta", "0 - 4");
    setCellStringValue("rng_Tau_e", "0 - 0.5");
    setCellStringValue("rng_g_a", "0.1 - 0.8");
    setCellStringValue("rng_p_1", "0 - 0.8");
    setCellStringValue("rng_p_2", "0 - 0.8");

    ui->tableWidget_uknown_params->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #0ecfe1;"  // фон
        "    color: black;"               // Белый текст
        "    font-weight: bold;"          // Жирный шрифт
        "    border: 2px solid #D1D1D1;"  // Граница
        "}");
    ui->doubleSpinBox_lambda_1->setValue(400);
    ui->doubleSpinBox_lambda_2->setValue(665);
}

AtmCorrectionMainWindow::~AtmCorrectionMainWindow() { delete ui; }

void AtmCorrectionMainWindow::setSunZenitAngle(const double value) {
    ui->doubleSpinBox_sunZenitAngle->setValue(value);
}

void AtmCorrectionMainWindow::setSunAzimutAngle(const double value) {
    ui->doubleSpinBox_sunAzimutAngle->setValue(value);
}

void AtmCorrectionMainWindow::setCaptureZenitAngle(const double value) {
    ui->doubleSpinBox_CaptureZenitAngle->setValue(value);
}

void AtmCorrectionMainWindow::setCaptureAzimutAngle(const double value) {
    ui->doubleSpinBox_CaptureAzimutAngle->setValue(value);
}

void AtmCorrectionMainWindow::on_pushButton_calculateBlack_clicked() {
    cs->setSunZenitAngle(ui->doubleSpinBox_sunZenitAngle->value());
    cs->setCaptruretZenitAngle(ui->doubleSpinBox_CaptureZenitAngle->value());
    cs->setFiAngle(ui->doubleSpinBox_sunAzimutAngle->value(),
                   ui->doubleSpinBox_CaptureAzimutAngle->value());
    cs->computeGamma();
    updateInitialValues();

    if (base_pixel_speya_values.size() == 10) {
        cs->start_solve_dark_pixels_async(
            ui->comboBox_satellite_type->currentText(),
            base_pixel_speya_values);
    } else {
        QMessageBox msgBox;
        msgBox.setText("Пиксель на снимке не выбран!");
        msgBox.setInformativeText("Выберите пиксель на снимке.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Пиксель не выбран!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    // qDebug() << bands_widget->get_choosed_bands(); первые 10 каналов пока
    // фиксируем
}

void AtmCorrectionMainWindow::showResult(result_values rv) {
    setCellValue("result_X", rv.X);
    setCellValue("result_q", rv.q);
    setCellValue("result_p", rv.p);
    setCellValue("result_Tau_m0", rv.tau_mu_0);
    setCellValue("result_Tau_a0", rv.tau_0_a);
    setCellValue("result_Beta", rv.beta);
    setCellValue("result_Tau_e", rv.tau_e);
    setCellValue("result_g_a", rv.g);
    setCellValue("result_p_1", rv.albedo_1);
    setCellValue("result_p_2", rv.albedo_2);
}

void AtmCorrectionMainWindow::on_comboBox_satellite_type_currentIndexChanged(
    const QString &arg1) {
    bands_widget->clear();
    QStringList sl;
    if (arg1 == "sentinel 2A") {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i)
            sl << sad::sentinel_2A_gui_band_names[i];
    } else if (arg1 == "sentinel 2B") {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i)
            sl << sad::sentinel_2B_gui_band_names[i];
    }
    bands_widget->updateCheckboxesList(sl);
    cs->updateCurrentSatellite(arg1);

    auto lambdas = cs->getLambdaList();
    auto responses = cs->getResponsesList();
    auto w = QVector<double>::fromStdVector(lambdas);

    for (size_t i = 0; i < responses.size(); ++i) {
        auto v = QVector<double>::fromStdVector(responses[i]);
        atm_params_plot->graph(i)->setData(w, v);
    }
    // auto h20 = QVector<double>::fromStdVector(cs->get_h2o());
    // atm_params_plot->graph(responses.size() - 1)->setData(w, h20);

    atm_params_plot->replot();
    atm_params_plot->rescaleAxes(true);
}

void AtmCorrectionMainWindow::updateBasePixel(QVector<double> pixel_bands) {
    if (pixel_bands.size() < 10) return;

    QString bands_values;
    base_pixel_speya_values.clear();
    for (int i = 0; i < 10; ++i) {
        base_pixel_speya_values.append(pixel_bands[i]);
        bands_values.append(QString::number(pixel_bands[i]));
        if (i < 10 - 1) bands_values.append(" ");
    }
    base_pixel_speya_values = {87.94741, 77.81722, 67.03251, 61.8863,
                               56.85879, 56.87943, 57.46667, 50.95204,
                               53.7519,  10.7995};  // TECT DELETE
    auto ln_m_H2O = cs->get_mH2O(10.7995, 53.7519);
    ui->label_pixel_bands->setText(bands_values);
    ui->doubleSpinBox_mH2O->setValue(std::exp(ln_m_H2O));
    auto a = cs->get_a_H2O();
    auto b = cs->get_b_H2O();
    auto w = cs->getLambdaList();
    Q_ASSERT(a.size() == b.size() == w.size());
    qDebug() << a.size() << b.size() << w.size();
    QVector<double> T_H2O;
    QVector<double> T_O3;
    for (size_t i = 0; i < w.size(); ++i) {
        T_H2O.append(a[i] * ln_m_H2O + b[i]);
    }
    atm_params_plot->graph(10)->setData(QVector<double>::fromStdVector(w),
                                        T_H2O);
    cs->setH2O(T_H2O);
    atm_params_plot->rescaleAxes();
    atm_params_plot->replot();
}

// Получение значения
double AtmCorrectionMainWindow::getCellValue(const QString &name) {
    // Проверяем, существует ли имя в нашей карте
    if (!cellMap.contains(name)) {
        qWarning() << "Имя ячейки не найдено в карте:" << name;
        return 0.0;
    }

    QPoint coords = cellMap[name];
    QTableWidgetItem *item =
        ui->tableWidget_uknown_params->item(coords.x(), coords.y());

    // Если элемент существует и не пустой, переводим строку в double
    if (item && !item->text().isEmpty()) {
        return item->text().toDouble();
    }

    return 0.0;  // Возвращаем 0, если ячейка пуста
}

// Установка значения
void AtmCorrectionMainWindow::setCellValue(const QString &name, double value,
                                           int precision) {
    // Проверяем наличие имени в карте
    if (!cellMap.contains(name)) {
        qWarning() << "Имя ячейки не найдено в карте для записи:" << name;
        return;
    }

    QPoint coords = cellMap[name];
    QTableWidgetItem *item =
        ui->tableWidget_uknown_params->item(coords.x(), coords.y());

    // Если ячейки физически нет в таблице, создаем её
    if (!item) {
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);  // Сохраняем центрирование
        ui->tableWidget_uknown_params->setItem(coords.x(), coords.y(), item);
    }

    // Преобразуем double в строку с фиксированным количеством знаков после
    // запятой ('f')
    item->setText(QString::number(value, 'f', precision));
}

void AtmCorrectionMainWindow::setCellStringValue(const QString &name,
                                                 const QString text) {
    // Проверяем наличие имени в карте
    if (!cellMap.contains(name)) {
        qWarning() << "Имя ячейки не найдено в карте для записи:" << name;
        return;
    }

    QPoint coords = cellMap[name];
    QTableWidgetItem *item =
        ui->tableWidget_uknown_params->item(coords.x(), coords.y());

    if (!item) {
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);  // Сохраняем центрирование
        ui->tableWidget_uknown_params->setItem(coords.x(), coords.y(), item);
    }

    item->setText(text);
}

void AtmCorrectionMainWindow::updateInitialValues() {
    cs->setInitial_values({getCellValue("X"), getCellValue("q"),
                           getCellValue("p"), getCellValue("Tau_m0"),
                           getCellValue("Tau_a0"), getCellValue("Beta"),
                           getCellValue("Tau_e"), getCellValue("g_a"),
                           getCellValue("p_1"), getCellValue("p_2")});
}
