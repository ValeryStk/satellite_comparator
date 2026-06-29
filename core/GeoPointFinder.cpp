#include "GeoPointFinder.h"

#include <QRegularExpression>

#include "ui_GeoPointFinder.h"

GeoPointFinder::GeoPointFinder(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::GeoPointFinder) {
    ui->setupUi(this);
}

GeoPointFinder::~GeoPointFinder() { delete ui; }

void GeoPointFinder::on_pushButton_find_result_clicked() {
    QString text = ui->textEdit_cordinates->toPlainText().trimmed();

    // Нормализуем: запятая или пробел как разделитель, точка как десятичный
    // знак
    text.replace(',', ' ');
    QStringList parts = text.split(QRegularExpression("\\s+"),
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                   Qt::SkipEmptyParts
#else
                                   QString::SkipEmptyParts
#endif
    );

    if (parts.size() < 2) {
        ui->label_parcedCord->clear();
        ui->textEdit_cordinates->setStyleSheet("border: 2px solid red;");
        return;
    }

    bool okLat = false, okLon = false;
    double lat = parts[0].toDouble(&okLat);
    double lon = parts[1].toDouble(&okLon);

    // Валидация диапазонов
    if (!okLat || !okLon || lat < -90.0 || lat > 90.0 || lon < -180.0 ||
        lon > 180.0) {
        ui->textEdit_cordinates->setStyleSheet("border: 2px solid red;");
        ui->label_parcedCord->clear();
        return;
    }
    ui->label_parcedCord->setText(
        QString("Вы ввели Ш: %1, Д: %2").arg(lat).arg(lon));
    ui->textEdit_cordinates->setStyleSheet("");  // сброс ошибки
    emit setGeoCoordinatesAsSample(QPointF(lat, lon));
}
