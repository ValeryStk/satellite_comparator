#include "slidersofparam.h"
#include "ui_SlidersOfParam.h"
#include <QDebug>
#include <QStyle>

constexpr int SATURATION_MAX_MULTIPLIER = 2;
constexpr int LIGHT_MAX_MULTIPLIER = 2;

SlidersOfParam::SlidersOfParam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SlidersOfParam)
{
    ui->setupUi(this);
    ui->label_light->setVisible(false);
    ui->label_sat->setVisible(false);
    ui->label_saturation_text->setPixmap(QPixmap::fromImage(QImage(":/res/saturation.svg")));
    ui->label_light_text->setPixmap(QPixmap::fromImage(QImage(":/res/light.svg")));
    ui->slider_sat->setValue(50);
    ui->slider_light->setValue(50);
    slider_sat = ui->slider_sat;
    slider_light = ui->slider_light;
    slider_sat->setRange(0,100);
    slider_light->setRange(0,100);

    qDebug()<<"slider_sat: "<<slider_sat->value() <<"slider_light: "<<slider_light->value();
    coefSat = 1.0+(slider_light->value()-50) / 50.0*(SATURATION_MAX_MULTIPLIER-1.0);
    coefLight = 1.0+(slider_light->value()-50) / 50.0*(LIGHT_MAX_MULTIPLIER-1.0);





//    connect(slider_sat, SIGNAL(sliderReleased()), this, SLOT(onSatChanged()));
//    connect(slider_light, SIGNAL(sliderReleased()), this, SLOT(onLightChanged()));

    connect(slider_sat, &QSlider::sliderReleased, this, [this]() {
        int value = slider_sat->value();
        qDebug() << "После перетягивания:" << value;

        onSatChanged();
    });

    connect(slider_sat, &QAbstractSlider::actionTriggered, this, [this](int action) {
        using AS = QAbstractSlider;
        if (action == AS::SliderPageStepAdd || action == AS::SliderPageStepSub) {
            int value = slider_sat->style()->sliderValueFromPosition(
                slider_sat->minimum(),
                slider_sat->maximum(),
                slider_sat->mapFromGlobal(QCursor::pos()).x(),
                slider_sat->width());
            slider_sat->setValue(value);
            qDebug() << "Клик по треку:" << value;

            onSatChanged();
        }
    });


    connect(slider_light, &QSlider::sliderReleased, this, [this]() {
        int value = slider_light->value();
        qDebug() << "После перетягивания:" << value;

        onLightChanged();
    });

    connect(slider_light, &QAbstractSlider::actionTriggered, this, [this](int action) {
        using AS = QAbstractSlider;
        if (action == AS::SliderPageStepAdd || action == AS::SliderPageStepSub) {
            int value = slider_light->style()->sliderValueFromPosition(
                slider_light->minimum(),
                slider_light->maximum(),
                slider_light->mapFromGlobal(QCursor::pos()).x(),
                slider_light->width());
            slider_light->setValue(value);
            qDebug() << "Клик по треку:" << value;

            onLightChanged();
        }
    });


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
    ui->slider_sat->setValue(50);
    ui->slider_light->setValue(50);
}

void SlidersOfParam::onSatChanged()
{
    ui->label_sat->setText(QString::number(ui->slider_sat->value()));
    coefSat = 1.0+(slider_sat->value()-50) / 50.0*(SATURATION_MAX_MULTIPLIER-1.0);
    emit slidersWereChanged();
}

void SlidersOfParam::onLightChanged()
{
    ui->label_light->setText(QString::number(ui->slider_light->value()));
    coefLight = 1.0+(slider_light->value()-50) / 50.0*(LIGHT_MAX_MULTIPLIER-1.0);
    emit slidersWereChanged();
}


