#include "AtmCorrectionMainWindow.h"

#include <QDebug>

#include "QStringList"
#include "satellites_structs.h"
#include "ui_AtmCorrectionMainWindow.h"

calculation_solver *cs;

AtmCorrectionMainWindow::AtmCorrectionMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AtmCorrectionMainWindow) {
    ui->setupUi(this);
    atm_params_plot = ui->widget_atm_params;
    ui->comboBox_satellite_type->blockSignals(true);
    ui->comboBox_satellite_type->addItems({"sentinel 2A", "sentinel 2B"});
    ui->comboBox_satellite_type->blockSignals(false);
    QStringList sl;
    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i)
        sl << sad::sentinel_2A_gui_band_names[i];
    bands_widget = new BandsWidget(sl, ui->verticalLayout_checkBoxes);
    // bands_widget->show();

    ui->doubleSpinBox_sunZenitAngle->setValue(45);
    ui->doubleSpinBox_black1->setValue(39.53);
    ui->doubleSpinBox_black2->setValue(25.64);
    ui->doubleSpinBox_black3->setValue(11.88);
    ui->doubleSpinBox_black4->setValue(4.31);
    qRegisterMetaType<result_values>();
    cs = new calculation_solver;
    connect(cs, &calculation_solver::darkpixels_calculation_finished, this,
            &AtmCorrectionMainWindow::showResult);
}

AtmCorrectionMainWindow::~AtmCorrectionMainWindow() { delete ui; }

void AtmCorrectionMainWindow::setSunZenitAngle(const double value) {
    ui->doubleSpinBox_sunZenitAngle->setValue(value);
}

void AtmCorrectionMainWindow::on_pushButton_calculateBlack_clicked() {
    cs->setSunZenitAngle(ui->doubleSpinBox_sunZenitAngle->value());
    cs->start_solve_dark_pixels_async(
        ui->comboBox_satellite_type->currentText(),
        {ui->doubleSpinBox_black1->value(), ui->doubleSpinBox_black2->value(),
         ui->doubleSpinBox_black3->value(), ui->doubleSpinBox_black4->value()});
    // qDebug() << bands_widget->get_choosed_bands(); первые 10 каналов
}

void AtmCorrectionMainWindow::showResult(result_values rv) {
    ui->doubleSpinBox_result_tau->setValue(rv.tau_0_a);
    ui->doubleSpinBox_result_beta->setValue(rv.beta);
    ui->doubleSpinBox_result_g->setValue(rv.g);
    ui->doubleSpinBox_result_albedo->setValue(rv.albedo);

    ui->doubleSpinBox_result_tau_error->setValue(rv.err_tau);
    ui->doubleSpinBox_result_beta_error->setValue(rv.err_beta);
    ui->doubleSpinBox_result_g_error->setValue(rv.err_g);
    ui->doubleSpinBox_result_albedo_error->setValue(rv.err_albedo);
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
}
