#include "main_window_satellite_comparator.h"
#include "ui_main_window_satellite_comparator.h"
#include "libs/gdal/x64/include/gdal.h"
#include "QDebug"

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , m_sat_comparator(new SatteliteComparator)
{
    ui->setupUi(this);
    qDebug()<<m_sat_comparator->get_satellites_data().keys();
}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
}

