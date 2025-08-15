#include "sliders_of_param.h"
#include "ui_sliders_of_param.h"
#include <QDebug>
#include <QStyle>

constexpr int SATURATION_MAX_MULTIPLIER = 2;
constexpr int LIGHT_MAX_MULTIPLIER = 2;
constexpr int SLIDER_INITIAL_VALUE = 50;
constexpr int SLIDER_MAX_VALUE = 100;

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
inline void calculate_sliders_coef(QSlider* slider_saturation,
                                   QSlider* slider_light,
                                   double& calculate_coef_saturation,
                                   double& calculate_coef_light){
    calculate_coef_saturation = 1.0 + (slider_saturation->value() - SLIDER_INITIAL_VALUE)
            / static_cast<double>(SLIDER_INITIAL_VALUE * (SATURATION_MAX_MULTIPLIER - 1.0));
    calculate_coef_light = 1.0 + (slider_light->value() - SLIDER_INITIAL_VALUE)
            / static_cast<double>(SLIDER_INITIAL_VALUE * (LIGHT_MAX_MULTIPLIER - 1.0));
}

// end of namespace

}

SlidersOfParam::SlidersOfParam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SlidersOfParam)
{
    ui->setupUi(this);
    ui->label_light->setVisible(false);
    ui->label_saturation->setVisible(false);
    ui->label_saturation_text->setPixmap(QPixmap::fromImage(QImage(":/res/saturation.svg")));
    ui->label_light_text->setPixmap(QPixmap::fromImage(QImage(":/res/light.svg")));
    ui->slider_saturation->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_light->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_saturation->setRange(0, SLIDER_MAX_VALUE);
    ui->slider_light->setRange(0, SLIDER_MAX_VALUE);
    calculate_sliders_coef(ui->slider_saturation, ui->slider_light, coefSaturation, coefLight);
    connect(ui->slider_saturation, SIGNAL(sliderReleased()), this, SLOT(onSaturationChanged()));
    connect(ui->slider_light, SIGNAL(sliderReleased()), this, SLOT(onLightChanged()));
}

SlidersOfParam::~SlidersOfParam()
{
    delete ui;
}

double SlidersOfParam::getCoefSaturation() const
{
    return coefSaturation;
}

double SlidersOfParam::getCoefLight() const
{
    return coefLight;
}

void SlidersOfParam::setDefaultValues()
{
    ui->slider_saturation->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_light->setValue(SLIDER_INITIAL_VALUE);
}

void SlidersOfParam::onSaturationChanged()
{
    calculate_sliders_coef(ui->slider_saturation, ui->slider_light, coefSaturation, coefLight);
    emit slidersWereChanged();
}

void SlidersOfParam::onLightChanged()
{
    calculate_sliders_coef(ui->slider_saturation, ui->slider_light, coefSaturation, coefLight);
    emit slidersWereChanged();
}

void SlidersOfParam::on_slider_light_actionTriggered(int action)
{
    if(get_slider_value_from_position(action,ui->slider_light)){
        onLightChanged();
    }
}

void SlidersOfParam::on_slider_saturation_actionTriggered(int action)
{
    if(get_slider_value_from_position(action, ui->slider_saturation)){
        onSaturationChanged();
    }
}

