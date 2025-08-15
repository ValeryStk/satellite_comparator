#ifndef SLIDERS_OF_PARAM_H
#define SLIDERS_OF_PARAM_H

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
    double coefSaturation;
    double coefLight;

};

#endif // SLIDERS_OF_PARAM_H
