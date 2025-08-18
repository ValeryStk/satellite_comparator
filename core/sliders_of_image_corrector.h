#ifndef SLIDERS_OF_IMAGE_CORRECTOR_H
#define SLIDERS_OF_IMAGE_CORRECTOR_H

#include <QWidget>
#include <QSlider>


constexpr double MAX_MULTIPLIER = 4;
constexpr double MIN_MULTIPLIER = 0.25;

constexpr int SLIDER_MAX_VALUE = 100;
constexpr int SLIDER_MIN_VALUE = 0;
constexpr int SLIDER_INITIAL_VALUE = (SLIDER_MAX_VALUE + SLIDER_MIN_VALUE)/2;



namespace Ui {
class SlidersOfImageCorrector;
}

class SlidersOfImageCorrector : public QWidget
{
    Q_OBJECT

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
