#ifndef GEOPOINTFINDER_H
#define GEOPOINTFINDER_H

#include <QMainWindow>

namespace Ui {
class GeoPointFinder;
}

class GeoPointFinder : public QMainWindow {
    Q_OBJECT

public:
    explicit GeoPointFinder(QWidget *parent = nullptr);
    ~GeoPointFinder();

private slots:
    void on_pushButton_find_result_clicked();

private:
    Ui::GeoPointFinder *ui;

signals:
    void setGeoCoordinatesAsSample(QPointF);
};

#endif  // GEOPOINTFINDER_H
