#include "UnitTests.h"

#include <QDebug>

#include "../../core/sattelite_comparator.h"



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
