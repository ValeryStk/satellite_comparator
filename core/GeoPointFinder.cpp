#include "GeoPointFinder.h"

#include "ui_GeoPointFinder.h"

GeoPointFinder::GeoPointFinder(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::GeoPointFinder) {
    ui->setupUi(this);
}

GeoPointFinder::~GeoPointFinder() { delete ui; }

void GeoPointFinder::on_pushButton_find_result_clicked() {
    setGeoCoordinatesAsSample(QPointF(ui->doubleSpinBox_latitude->value(),
                                      ui->doubleSpinBox_longitude->value()));
}
