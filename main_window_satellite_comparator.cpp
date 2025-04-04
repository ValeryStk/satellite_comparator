#include "main_window_satellite_comparator.h"
#include "ui_main_window_satellite_comparator.h"

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
{
    ui->setupUi(this);
}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
}

