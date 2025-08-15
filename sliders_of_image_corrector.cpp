#include "sliders_of_image_corrector.h"
#include "ui_sliders_of_image_corrector.h"

#include <QStyle>
#include <QtGlobal>
#include <QDebug>


constexpr double MAX_MULTIPLIER = 4;
constexpr double MIN_MULTIPLIER = 0.25;

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

inline double calculate_slider_coef(QSlider* slider){
    Q_ASSERT(MIN_MULTIPLIER < 1.0);
    Q_ASSERT(MAX_MULTIPLIER > 1.0);
    int sVal = slider->value();
    if (sVal == SLIDER_INITIAL_VALUE) return 1.0;
    if (sVal < SLIDER_INITIAL_VALUE) {
        double t = static_cast<double>(SLIDER_INITIAL_VALUE - sVal) / (SLIDER_INITIAL_VALUE - SLIDER_MIN_VALUE);
        return 1.0 - t * (1.0 - MIN_MULTIPLIER);
    }
    if (sVal > SLIDER_INITIAL_VALUE) {
        double t = static_cast<double>(sVal - SLIDER_INITIAL_VALUE) / (SLIDER_MAX_VALUE - SLIDER_INITIAL_VALUE);
        return 1.0 + t * (MAX_MULTIPLIER - 1.0);
    }
    return -1.0; // Невозможное значение, но на всякий случай
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
    coefSaturation = 1;
    coefLight = 1;
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
