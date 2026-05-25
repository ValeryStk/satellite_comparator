#include "AtmCorrectionMainWindow.h"

#include <QDebug>

#include "QStringList"
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
    cs = new calculation_solver;
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
        atm_params_plot->graph(i)->setPen(QPen(channelColors[i]));
        atm_params_plot->graph(i)->setData(
            w, QVector<double>::fromStdVector(responses[i]));
    }
    atm_params_plot->addGraph();  // для Т_H20
    atm_params_plot->graph(10)->setPen(QPen("#0ecfe1"));

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
    /*cs->start_solve_dark_pixels_async(
        ui->comboBox_satellite_type->currentText(),
        {ui->doubleSpinBox_black1->value(), ui->doubleSpinBox_black2->value(),
         ui->doubleSpinBox_black3->value(),
       ui->doubleSpinBox_black4->value()});*/
    // qDebug() << bands_widget->get_choosed_bands(); первые 10 каналов
}

void AtmCorrectionMainWindow::showResult(result_values rv) {
    /*ui->doubleSpinBox_result_tau->setValue(rv.tau_0_a);
    ui->doubleSpinBox_result_beta->setValue(rv.beta);
    ui->doubleSpinBox_result_g->setValue(rv.g);
    ui->doubleSpinBox_result_albedo->setValue(rv.albedo);

    ui->doubleSpinBox_result_tau_error->setValue(rv.err_tau);
    ui->doubleSpinBox_result_beta_error->setValue(rv.err_beta);
    ui->doubleSpinBox_result_g_error->setValue(rv.err_g);
    ui->doubleSpinBox_result_albedo_error->setValue(rv.err_albedo);*/
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
    QString bands_values;
    for (int i = 0; i < pixel_bands.size(); ++i) {
        bands_values.append(QString::number(pixel_bands[i]));
        if (i < pixel_bands.size() - 1) bands_values.append(" ");
    }
    base_pixel_speya_values = pixel_bands;
    auto ln_m_H2O = cs->get_mH2O(pixel_bands[9], pixel_bands[8]);
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
    atm_params_plot->rescaleAxes();
    atm_params_plot->replot();
}
