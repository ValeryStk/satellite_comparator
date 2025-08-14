#ifndef SLIDERSOFPARAM_H
#define SLIDERSOFPARAM_H

#include <QWidget>
#include <QSlider>

namespace Ui {
class SlidersOfParam;
}

class SlidersOfParam : public QWidget
{
    Q_OBJECT

public:
    explicit SlidersOfParam(QWidget *parent = nullptr);
    ~SlidersOfParam();
    double getCoef1() const;
    double getCoef2() const;
    void setDefaultValues();



signals:
    void slidersWereChanged();

private slots:
    void onSatChanged();
    void onLightChanged();

private:
    Ui::SlidersOfParam *ui;
    QSlider *slider_sat;
    QSlider *slider_light;
    double coefSat;
    double coefLight;


};



#endif // SLIDERSOFPARAM_H
