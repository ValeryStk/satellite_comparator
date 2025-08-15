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
    double getCoefSaturation() const;
    double getCoefLight() const;
    void setDefaultValues();

signals:
    void slidersWereChanged();

private slots:
    void onSaturationChanged();
    void onLightChanged();
    void on_slider_light_actionTriggered(int action);
    void on_slider_saturation_actionTriggered(int action);

private:
    Ui::SlidersOfParam *ui;
    QSlider *slider_saturation;
    QSlider *slider_light;
    double coefSaturation;
    double coefLight;

};

#endif // SLIDERSOFPARAM_H
