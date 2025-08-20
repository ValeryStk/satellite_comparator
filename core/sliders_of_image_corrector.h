#ifndef SLIDERS_OF_IMAGE_CORRECTOR_H
#define SLIDERS_OF_IMAGE_CORRECTOR_H

#include <QWidget>
#include <QSlider>


namespace Ui {
class SlidersOfImageCorrector;
}

class SlidersOfImageCorrector : public QWidget
{
    Q_OBJECT
    friend class UnitTests;

public:
    explicit SlidersOfImageCorrector(QWidget *parent = nullptr);
    ~SlidersOfImageCorrector();
    double getCoefSaturation() const;
    double getCoefLight() const;
    void setDefaultValues();
    QSlider* getLightSlider();
    QSlider* getSaturationSlider();

signals:
    void slidersWereChanged();

private slots:
    void onSaturationChanged();
    void onLightChanged();
    void on_slider_light_actionTriggered(int action);
    void on_slider_saturation_actionTriggered(int action);

private:
    Ui::SlidersOfImageCorrector *ui;
    double coefSaturation;
    double coefLight;

};

#endif // SLIDERS_OF_IMAGE_CORRECTOR_H
