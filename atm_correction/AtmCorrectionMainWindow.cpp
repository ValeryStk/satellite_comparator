#include "AtmCorrectionMainWindow.h"

#include <QDebug>

#include "json_utils.h"
#include "ui_AtmCorrectionMainWindow.h"
calculation_solver *cs;

AtmCorrectionMainWindow::AtmCorrectionMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AtmCorrectionMainWindow) {
    ui->setupUi(this);
    ui->doubleSpinBox_sunZenitAngle->setValue(45);
    ui->doubleSpinBox_black1->setValue(39.53);
    ui->doubleSpinBox_black2->setValue(25.64);
    ui->doubleSpinBox_black3->setValue(11.88);
    ui->doubleSpinBox_black4->setValue(4.31);
    qRegisterMetaType<result_values>();
    QJsonArray jarr;
    jsn::getJsonArrayFromFile(":/sdb.json", jarr);
    cs = new calculation_solver;
    connect(cs, &calculation_solver::darkpixels_calculation_finished, this,
            &AtmCorrectionMainWindow::showResult);
}

AtmCorrectionMainWindow::~AtmCorrectionMainWindow() { delete ui; }

void AtmCorrectionMainWindow::on_pushButton_calculateBlack_clicked() {
    // sentinel2a-20m
    cs->setElavationAngle(90 - ui->doubleSpinBox_sunZenitAngle->value());
    cs->start_solve_dark_pixels_async(
        "sentinel2a-10m",
        {ui->doubleSpinBox_black1->value(), ui->doubleSpinBox_black2->value(),
         ui->doubleSpinBox_black3->value(), ui->doubleSpinBox_black4->value()});
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
