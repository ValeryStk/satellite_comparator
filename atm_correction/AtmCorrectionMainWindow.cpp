#include "AtmCorrectionMainWindow.h"

#include <QDebug>

#include "json_utils.h"
#include "ui_AtmCorrectionMainWindow.h"

AtmCorrectionMainWindow::AtmCorrectionMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AtmCorrectionMainWindow) {
    ui->setupUi(this);
    QJsonObject jo;
    jsn::getJsonObjectFromFile(":/sd.json", jo);
    qDebug() << jo.keys();
}

AtmCorrectionMainWindow::~AtmCorrectionMainWindow() { delete ui; }
