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

void UnitTests::helper_for_test_coef(int slider_value,
                                     double multiplier_value,
                                     const char* name_test,
                                     SlidersOfImageCorrector* sic){

    auto slider_saturation = sic->getSaturationSlider();
    auto slider_light = sic->getLightSlider();

    slider_light->setValue(slider_value);
    slider_saturation->setValue(slider_value);

    sic->onLightChanged();
    sic->onSaturationChanged();

    auto coef_sat=sic->getCoefSaturation();
    auto coef_light=sic->getCoefLight();

    qDebug()<<coef_sat<<"coef sat: "<<coef_light<<"coef light: ";
    qDebug()<<multiplier_value<<"MULTIPL";
    QVERIFY2(multiplier_value==coef_sat,name_test);
    QVERIFY2(multiplier_value==coef_light,name_test);

}

void UnitTests::testSliderImageCorrector()
{
    qDebug()<<"----------TEST SLIDER IMAGE CORRECTOR---------\n";
    auto sic = std::make_unique<SlidersOfImageCorrector>();
    auto slider_saturation = sic->getSaturationSlider();
    auto slider_light = sic->getLightSlider();

    QVERIFY2(sic->getCoefLight()==1,"Initial value for light slider");
    QVERIFY2(sic->getCoefSaturation()==1,"Initial value for saturation slider");
    QVERIFY2(slider_saturation->maximum()==SLIDER_MAX_VALUE,"MAX value for saturation slider");
    QVERIFY2(slider_light->maximum()==SLIDER_MAX_VALUE,"MAX value for light slider");
    QVERIFY2(slider_saturation->minimum()==SLIDER_MIN_VALUE,"MIN value for saturation slider" );
    QVERIFY2(slider_light->minimum()==SLIDER_MIN_VALUE,"MIN value for light slider");


    helper_for_test_coef(SLIDER_MAX_VALUE,
                         calculate_proportion_coefficient(SLIDER_MAX_VALUE,{},{}),
                         "coef value for MAX MULTIPLIER",
                         sic.get());

    helper_for_test_coef(SLIDER_INITIAL_VALUE,
                         calculate_proportion_coefficient(SLIDER_INITIAL_VALUE,{},{}),
                         "coef value for INITIAL MULTIPLIER",
                         sic.get());

    helper_for_test_coef(SLIDER_MIN_VALUE,
                         calculate_proportion_coefficient(SLIDER_MIN_VALUE,{},{}),
                         "coef value for MIN MULTIPLIER",
                         sic.get());

    helper_for_test_coef(SLIDER_MAX_VALUE*3/4,
                         calculate_proportion_coefficient(SLIDER_MAX_VALUE*3/4,{},{}),
                         "coef value",
                         sic.get());

    helper_for_test_coef(SLIDER_MAX_VALUE*1/4,
                         calculate_proportion_coefficient(SLIDER_MAX_VALUE*1/4,{},{}),
                         "coef value",
                         sic.get());

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
