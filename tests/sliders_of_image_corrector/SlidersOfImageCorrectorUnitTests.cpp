#include "SlidersOfImageCorrectorUnitTests.h"

#include <QDebug>
#include <QVector>
#include "../../core/sliders_of_image_corrector.cpp"


namespace{


} // end namespace


SlidersOfImageCorrectorUnitTests::SlidersOfImageCorrectorUnitTests()
{
}

void SlidersOfImageCorrectorUnitTests::initTestCase()
{
    // Инициализация перед запуском всех тестов
}

void SlidersOfImageCorrectorUnitTests::cleanupTestCase()
{
    // Очистка после выполнения всех тестов
}

void SlidersOfImageCorrectorUnitTests::init()
{
    // Инициализация перед каждым тестом

}

void SlidersOfImageCorrectorUnitTests::cleanup()
{
    // Очистка после каждого теста
}

void SlidersOfImageCorrectorUnitTests::helper_for_test_coef(const int slider_value,
                                     const double expected_multiplier_value,
                                     const QString& name_test,
                                     SlidersOfImageCorrector* sic,
                                     const uint header_width){

    auto slider_saturation = sic->getSaturationSlider();
    auto slider_light = sic->getLightSlider();

    slider_light->setValue(slider_value);
    slider_saturation->setValue(slider_value);

    sic->onLightChanged();
    sic->onSaturationChanged();

    auto coef_sat = sic->getCoefSaturation();
    auto coef_light = sic->getCoefLight();


    qDebug().nospace()          << qSetFieldWidth(header_width) <<slider_value
                       << " | " << qSetFieldWidth(header_width) << coef_sat
                       << " | " << qSetFieldWidth(header_width) << coef_light
                       << " | " << qSetFieldWidth(header_width) << expected_multiplier_value;
    qDebug().resetFormat();

    QVERIFY2(expected_multiplier_value == coef_sat,   name_test.toLocal8Bit());
    QVERIFY2(expected_multiplier_value == coef_light, name_test.toLocal8Bit());

}

void SlidersOfImageCorrectorUnitTests::testSliderImageCorrector()
{
    qDebug()<<"----------TEST SLIDER IMAGE CORRECTOR---------\n";
    auto sic = std::make_unique<SlidersOfImageCorrector>();
    auto slider_saturation = sic->getSaturationSlider();
    auto slider_light = sic->getLightSlider();

    QVERIFY2(sic->getCoefLight()          == NEUTRAL_MULTIPLIER,"Initial value for light slider");
    QVERIFY2(sic->getCoefSaturation()     == NEUTRAL_MULTIPLIER,"Initial value for saturation slider");
    QVERIFY2(slider_saturation->maximum() == SLIDER_MAX_VALUE,  "MAX value for saturation slider");
    QVERIFY2(slider_light->maximum()      == SLIDER_MAX_VALUE,  "MAX value for light slider");
    QVERIFY2(slider_saturation->minimum() == SLIDER_MIN_VALUE,  "MIN value for saturation slider" );
    QVERIFY2(slider_light->minimum()      == SLIDER_MIN_VALUE,  "MIN value for light slider");

    const uint width = 6;
    qDebug().nospace()          << qSetFieldWidth(width) << "value"
                       << " | " << qSetFieldWidth(width) << "sc"
                       << " | " << qSetFieldWidth(width) << "lc"
                       << " | " << qSetFieldWidth(width) << "er";

    struct test_params{
        double slider_value;
        double expected_multiplier;
        QString error_message;
    };

    QString wrong_message = "WRONG coef value for ";
                                   // Slider value           // Expected result  // Error message
    QVector<test_params> params = {{SLIDER_MAX_VALUE,        MAX_MULTIPLIER,     wrong_message + "MAX MULTIPLIER"},
                                   {SLIDER_INITIAL_VALUE,    NEUTRAL_MULTIPLIER, wrong_message + "INITIAL MULTIPLIER"},
                                   {SLIDER_MIN_VALUE,        MIN_MULTIPLIER,     wrong_message + "MIN MULTIPLIER"},
                                   {SLIDER_MAX_VALUE * 0.75, 2.5,                wrong_message + "0.75"},
                                   {SLIDER_MAX_VALUE * 0.25, 0.625,              wrong_message + "0.25"}
                                  };

    for(int i=0;i<params.size();++i){
       auto p = params[i];
        helper_for_test_coef(p.slider_value,
                             p.expected_multiplier,
                             p.error_message,sic.get(),
                             width);
    }
}


QTEST_MAIN(SlidersOfImageCorrectorUnitTests)
