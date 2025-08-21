#include "UnitTests.h"

#include <QDebug>
#include <QVector>
#include "../core/sliders_of_image_corrector.cpp"
#include "../core/sattelite_comparator.h"
#include "../core/json_utils.cpp"

namespace{


} // end namespace


UnitTests::UnitTests()
{
}

void UnitTests::initTestCase()
{
    // Инициализация перед запуском всех тестов
}

void UnitTests::cleanupTestCase()
{
    // Очистка после выполнения всех тестов
}

void UnitTests::init()
{
    // Инициализация перед каждым тестом

}

void UnitTests::cleanup()
{
    // Очистка после каждого теста
}

void UnitTests::helper_for_test_coef(const int slider_value,
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

void UnitTests::testSliderImageCorrector()
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

void UnitTests::testSatelliteComparatorBaseCheck()
{
    qDebug()<<"----------TEST SATELLITE COMPARATOR BASE CHECK---------\n";
    auto sc = std::make_unique<SatteliteComparator>();
    QVERIFY2( BASE_CHECK_RESULT::OK==
              sc->base_check_before_interpolation({1,2,3},{1,2,3}),
              "Check OK result");
    QVERIFY2(BASE_CHECK_RESULT::WAVES_IS_EMPTY ==
             sc->base_check_before_interpolation({},{1,2,3}),
             "Check empty wave1 result");
    QVERIFY2(BASE_CHECK_RESULT::VALUES_IS_EMPTY ==
             sc->base_check_before_interpolation({1,2,3},{}),
             "Check empty wave2 result");
    QVERIFY2(BASE_CHECK_RESULT::SIZES_ARE_NOT_THE_SAME ==
             sc->base_check_before_interpolation({1,2,3},{1,2,3,4,5}),
             "Sizes are not the same");
    QVERIFY2(BASE_CHECK_RESULT::SIZES_ARE_NOT_THE_SAME ==
             sc->base_check_before_interpolation({1,2,3,5,6,7},{1,2,3}),
             "Sizes are not the same");
    QVERIFY2(BASE_CHECK_RESULT::WAVES_IS_NOT_SORTED ==
             sc->base_check_before_interpolation({10,9,8,7},{1,2,3,4}),
             "Check not sorted wave1 result");
}

void UnitTests::testSatelliteComparatorLinearInterpolation()
{
    auto sc = std::make_unique<SatteliteComparator>();
    BASE_CHECK_RESULT status;
    auto result = sc->interpolate({1,2,3,4,5},
                                  {1.5,2.6,3.7,4.8,5.6},
                                  {1.2,2.2,3.4,4.5,4.6},
                                  status);
    QVERIFY2(status == BASE_CHECK_RESULT::OK,"Correct interpolation params");
}


QTEST_MAIN(UnitTests)
