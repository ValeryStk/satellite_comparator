#include "sliders_of_image_corrector.h"
#include "ui_sliders_of_image_corrector.h"

#include <QStyle>
#include <QtGlobal>
#include <QDebug>

constexpr double MAX_MULTIPLIER = 4;
constexpr double MIN_MULTIPLIER = 0.25;
constexpr double NEUTRAL_MULTIPLIER = 1.0;

constexpr int SLIDER_MAX_VALUE = 100;
constexpr int SLIDER_MIN_VALUE = 0;
constexpr int SLIDER_INITIAL_VALUE = (SLIDER_MAX_VALUE + SLIDER_MIN_VALUE)/2;

namespace {

inline bool get_slider_value_from_position(const int action,
                                           QSlider* slider){
    using AS = QAbstractSlider;
    if (action == AS::SliderPageStepAdd || action == AS::SliderPageStepSub) {
        int value = slider->style()->sliderValueFromPosition(
                    slider->minimum(),
                    slider->maximum(),
                    slider->mapFromGlobal(QCursor::pos()).x(),
                    slider->width());
        slider->setValue(value);
        return true;
    }
    return false;
}


inline double calculate_proportion_coefficient(const int base_range_value,
                                               std::pair<double,double> base_range,
                                               std::pair<double,double> multiplier_range){

    Q_UNUSED(base_range)
    Q_UNUSED(multiplier_range)

    if (base_range_value == SLIDER_INITIAL_VALUE) return NEUTRAL_MULTIPLIER;
    if (base_range_value < SLIDER_INITIAL_VALUE) {
        double t = static_cast<double>(SLIDER_INITIAL_VALUE - base_range_value) /
                (SLIDER_INITIAL_VALUE - SLIDER_MIN_VALUE);
        return NEUTRAL_MULTIPLIER - t * (NEUTRAL_MULTIPLIER - MIN_MULTIPLIER);
    }
    if (base_range_value > SLIDER_INITIAL_VALUE) {
        double t = static_cast<double>(base_range_value - SLIDER_INITIAL_VALUE) /
                (SLIDER_MAX_VALUE - SLIDER_INITIAL_VALUE);
        return NEUTRAL_MULTIPLIER + t * (MAX_MULTIPLIER - NEUTRAL_MULTIPLIER);
    }
    return -1.0; //Unexpected result
}

inline double calculate_slider_coef(const QSlider* slider){
    Q_ASSERT(MIN_MULTIPLIER < NEUTRAL_MULTIPLIER);
    Q_ASSERT(MAX_MULTIPLIER > NEUTRAL_MULTIPLIER);
    return calculate_proportion_coefficient(slider->value(),
                                            {SLIDER_MIN_VALUE,SLIDER_MAX_VALUE},
                                            {MIN_MULTIPLIER,MAX_MULTIPLIER});
}
// end of namespace

}

SlidersOfImageCorrector::SlidersOfImageCorrector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SlidersOfImageCorrector)
{
    ui->setupUi(this);
    ui->label_light->setVisible(false);
    ui->label_saturation->setVisible(false);
    ui->label_saturation_text->setPixmap(QPixmap::fromImage(QImage(":/res/saturation.svg")));
    ui->label_light_text->setPixmap(QPixmap::fromImage(QImage(":/res/light.svg")));
    ui->slider_saturation->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_light->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_saturation->setRange(SLIDER_MIN_VALUE, SLIDER_MAX_VALUE);
    ui->slider_light->setRange(SLIDER_MIN_VALUE, SLIDER_MAX_VALUE);
    coefSaturation = NEUTRAL_MULTIPLIER;
    coefLight = NEUTRAL_MULTIPLIER;
    connect(ui->slider_saturation, SIGNAL(sliderReleased()), this, SLOT(onSaturationChanged()));
    connect(ui->slider_light, SIGNAL(sliderReleased()), this, SLOT(onLightChanged()));
}

SlidersOfImageCorrector::~SlidersOfImageCorrector()
{
    delete ui;
}

double SlidersOfImageCorrector::getCoefSaturation() const
{
    return coefSaturation;
}

double SlidersOfImageCorrector::getCoefLight() const
{
    return coefLight;
}

void SlidersOfImageCorrector::setDefaultValues()
{
    ui->slider_saturation->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_light->setValue(SLIDER_INITIAL_VALUE);
    coefSaturation = NEUTRAL_MULTIPLIER;
    coefLight = NEUTRAL_MULTIPLIER;
}

QSlider *SlidersOfImageCorrector::getLightSlider()
{
    return ui->slider_light;
}

QSlider *SlidersOfImageCorrector::getSaturationSlider()
{
    return ui->slider_saturation;
}

void SlidersOfImageCorrector::onSaturationChanged()
{
    coefSaturation = calculate_slider_coef(ui->slider_saturation);
    emit slidersWereChanged();
}

void SlidersOfImageCorrector::onLightChanged()
{
    coefLight = calculate_slider_coef(ui->slider_light);
    emit slidersWereChanged();
}

void SlidersOfImageCorrector::on_slider_light_actionTriggered(int action)
{
    if(get_slider_value_from_position(action,ui->slider_light)){
        onLightChanged();
    }
}

void SlidersOfImageCorrector::on_slider_saturation_actionTriggered(int action)
{
    if(get_slider_value_from_position(action, ui->slider_saturation)){
        onSaturationChanged();
    }
}
