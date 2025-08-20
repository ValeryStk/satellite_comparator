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
                                      const char* name_test){

    SlidersOfImageCorrector* sic = new SlidersOfImageCorrector;
    sic->setAttribute(Qt::WA_DeleteOnClose);

    auto slider_saturation = sic->getSaturationSlider();
    auto slider_light = sic->getLightSlider();

    slider_light->setValue(slider_value);
    slider_saturation->setValue(slider_value);

    sic->onLightChanged();
    sic->onSaturationChanged();

    QTest::mouseClick(slider_light, Qt::LeftButton);
    auto coef_sat=sic->getCoefSaturation();
    auto coef_light=sic->getCoefLight();

    qDebug()<<coef_sat<<"coef sat: "<<coef_light<<"coef light: ";
    QVERIFY2(multiplier_value==coef_sat,name_test);
    QVERIFY2(multiplier_value==coef_light,name_test);

}

void UnitTests::testSliderImageCorrector()
{
    qDebug()<<"----------TEST SLIDER IMAGE CORRECTOR---------\n";
        SlidersOfImageCorrector* sic = new SlidersOfImageCorrector;
        sic->setAttribute(Qt::WA_DeleteOnClose);
        auto slider_saturation = sic->getSaturationSlider();
        auto slider_light = sic->getLightSlider();

        QVERIFY2(sic->getCoefLight()==1,"Initial value for light slider");
        QVERIFY2(sic->getCoefSaturation()==1,"Initial value for saturation slider");
        QVERIFY2(slider_saturation->maximum()==SLIDER_MAX_VALUE,"MAX value for saturation slider");
        QVERIFY2(slider_light->maximum()==SLIDER_MAX_VALUE,"MAX value for light slider");
        QVERIFY2(slider_saturation->minimum()==SLIDER_MIN_VALUE,"MIN value for saturation slider" );
        QVERIFY2(slider_light->minimum()==SLIDER_MIN_VALUE,"MIN value for light slider");


        helper_for_test_coef(SLIDER_MAX_VALUE,MAX_MULTIPLIER,"MAX");
        helper_for_test_coef(SLIDER_MIN_VALUE,MIN_MULTIPLIER,"MIN");

//        slider_light->setValue(SLIDER_MAX_VALUE);
//        slider_saturation->setValue(SLIDER_MAX_VALUE);
//        sic->onLightChanged();
//        sic->onSaturationChanged();
//        QTest::mouseClick(slider_light, Qt::LeftButton);
//        auto coef_sat=sic->getCoefSaturation();
//        auto coef_light=sic->getCoefLight();

//        qDebug()<<coef_sat<<"coef sat: "<<coef_light<<"coef light: ";
//        QVERIFY2(MAX_MULTIPLIER==coef_sat,"coef sat value for max MULTIPLIER");
//        QVERIFY2(MAX_MULTIPLIER==coef_light,"coef light value for max MULTIPLIER");

//        slider_light->setValue(SLIDER_MIN_VALUE);
//        slider_saturation->setValue(SLIDER_MIN_VALUE);
//        sic->onLightChanged();
//        sic->onSaturationChanged();
//        auto coef_sat2=sic->getCoefSaturation();
//        auto coef_light2=sic->getCoefLight();

//        qDebug()<<"coef_sat"<<coef_sat2<<"coef_light"<<coef_light2;
//        QVERIFY2(MIN_MULTIPLIER==coef_sat2,"coef sat2 value for min MULTIPLIER");
//        QVERIFY2(MIN_MULTIPLIER==coef_light2,"coef light2 value for min MULTIPLIER");

//        slider_light->setValue(SLIDER_MAX_VALUE-SLIDER_INITIAL_VALUE/2);
//        slider_saturation->setValue(SLIDER_MAX_VALUE-SLIDER_INITIAL_VALUE/2);
//        sic->onLightChanged();
//        sic->onSaturationChanged();
//        auto coef_sat3=sic->getCoefSaturation();
//        auto coef_light3=sic->getCoefLight();

//        qDebug()<<"coef_sat"<<coef_sat3<<"coef_light"<<coef_light3;
//        QVERIFY2(2.5==coef_sat3,"coef sat3 value for max MULTIPLIER");
//        QVERIFY2(2.5==coef_light3,"coef light3 value for max MULTIPLIER");

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
