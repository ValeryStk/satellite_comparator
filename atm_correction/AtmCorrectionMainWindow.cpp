#include "AtmCorrectionMainWindow.h"

#include <QDebug>

#include "calculation_solver.h"
#include "json_utils.h"
#include "ui_AtmCorrectionMainWindow.h"
calculation_solver *cs;

AtmCorrectionMainWindow::AtmCorrectionMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AtmCorrectionMainWindow) {
    ui->setupUi(this);
    QJsonArray jarr;
    jsn::getJsonArrayFromFile(":/sdb.json", jarr);
    cs = new calculation_solver;
}

AtmCorrectionMainWindow::~AtmCorrectionMainWindow() { delete ui; }

void AtmCorrectionMainWindow::on_pushButton_calculateBlack_clicked() {
    // sentinel2a-20m
    cs->start_solve_dark_pixels_async(
        "sentinel2a-10m",
        {ui->doubleSpinBox_black1->value(), ui->doubleSpinBox_black2->value(),
         ui->doubleSpinBox_black3->value(), ui->doubleSpinBox_black4->value()});
}
