#include "UnitTests.h"

#include <QDebug>
#include "../core/sliders_of_image_corrector.h"

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
    //slider_saturation->maximum()

    slider_saturation->setValue(slider_saturation->maximum());

}


QTEST_MAIN(UnitTests)
