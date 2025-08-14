#include "slidersofparam.h"
#include "ui_slidersofparam.h"
#include <QDebug>
#include <QStyle>

constexpr int SATURATION_MAX_MULTIPLIER = 2;
constexpr int LIGHT_MAX_MULTIPLIER = 2;
constexpr int SLIDER_INITIAL_VALUE = 50;
constexpr int SLIDER_MAX_VALUE = 100;

namespace {

inline bool get_slider_value_from_position(const int action, QSlider* slider){
    using AS = QAbstractSlider;
    if (action == AS::SliderPageStepAdd || action == AS::SliderPageStepSub) {
        int value = slider->style()->sliderValueFromPosition(
                    slider->minimum(),
                    slider->maximum(),
                    slider->mapFromGlobal(QCursor::pos()).x(),
                    slider->width());
        slider->setValue(value);
        return true;
    };
    return false;
}
// end of namespace

}

SlidersOfParam::SlidersOfParam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SlidersOfParam)
{
    ui->setupUi(this);
    ui->label_light->setVisible(false);
    ui->label_sat->setVisible(false);
    ui->label_saturation_text->setPixmap(QPixmap::fromImage(QImage(":/res/saturation.svg")));
    ui->label_light_text->setPixmap(QPixmap::fromImage(QImage(":/res/light.svg")));
    ui->slider_sat->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_light->setValue(SLIDER_INITIAL_VALUE);
    slider_sat = ui->slider_sat;
    slider_light = ui->slider_light;
    slider_sat->setRange(0, SLIDER_MAX_VALUE);
    slider_light->setRange(0, SLIDER_MAX_VALUE);
    coefSat = 1.0 + (slider_light->value() - SLIDER_INITIAL_VALUE)
            / static_cast<double>(SLIDER_INITIAL_VALUE * (SATURATION_MAX_MULTIPLIER - 1.0));
    coefLight = 1.0 + (slider_light->value() - SLIDER_INITIAL_VALUE)
            / static_cast<double>(SLIDER_INITIAL_VALUE * (LIGHT_MAX_MULTIPLIER - 1.0));
    connect(slider_sat, SIGNAL(sliderReleased()), this, SLOT(onSatChanged()));
    connect(slider_light, SIGNAL(sliderReleased()), this, SLOT(onLightChanged()));
}

SlidersOfParam::~SlidersOfParam()
{
    delete ui;
}

double SlidersOfParam::getCoef1() const
{
    return coefSat;
}

double SlidersOfParam::getCoef2() const
{
    return coefLight;
}

void SlidersOfParam::setDefaultValues()
{
    ui->slider_sat->setValue(SLIDER_INITIAL_VALUE);
    ui->slider_light->setValue(SLIDER_INITIAL_VALUE);
}

void SlidersOfParam::onSatChanged()
{
    coefSat = 1.0 + (slider_sat->value() - SLIDER_INITIAL_VALUE)
            / static_cast<double>(SLIDER_INITIAL_VALUE * (SATURATION_MAX_MULTIPLIER - 1.0));
    emit slidersWereChanged();
}

void SlidersOfParam::onLightChanged()
{
    coefLight = 1.0 + (slider_light->value() - SLIDER_INITIAL_VALUE)
            / static_cast<double>(SLIDER_INITIAL_VALUE * (LIGHT_MAX_MULTIPLIER - 1.0));
    emit slidersWereChanged();
}

void SlidersOfParam::on_slider_light_actionTriggered(int action)
{
    if(get_slider_value_from_position(action,slider_light)){
        onLightChanged();
    }
}

void SlidersOfParam::on_slider_sat_actionTriggered(int action)
{
    if(get_slider_value_from_position(action, slider_sat)){
        onSatChanged();
    }
}
