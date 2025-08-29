#include "SatelliteComparatorUnitTests.h"

#include <QDebug>
#include "davis.h"
#include "sattelite_comparator.h"



namespace{


} // end namespace


SatelliteComparatorUnitTests::SatelliteComparatorUnitTests()
{
}

void SatelliteComparatorUnitTests::initTestCase()
{
    // Инициализация перед запуском всех тестов
}

void SatelliteComparatorUnitTests::cleanupTestCase()
{
    // Очистка после выполнения всех тестов
}

void SatelliteComparatorUnitTests::init()
{
    // Инициализация перед каждым тестом

}

void SatelliteComparatorUnitTests::cleanup()
{
    // Очистка после каждого теста
}

void SatelliteComparatorUnitTests::testSatelliteComparatorBaseCheck()
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

void SatelliteComparatorUnitTests::testSatelliteComparatorLinearInterpolation()
{
    auto sc = std::make_unique<SatteliteComparator>();
    BASE_CHECK_RESULT status;
    auto result = sc->interpolate({1,2,3,4,5},
                                  {1.5,2.6,3.7,4.8,5.6},
                                  {1.2,2.2,3.4,4.5,4.6},
                                  status);
    QVERIFY2(status == BASE_CHECK_RESULT::OK,"Correct interpolation params");
}

void SatelliteComparatorUnitTests::testLoadSatelliteDataJson()
{
    auto sc = std::make_unique<SatteliteComparator>();
    //sc->loadJsonSatellitesCentralWaves();
    auto sd = sc->get_satellites_data();
    qDebug()<<"sd size: "<<sd.size();
    auto json_object = sc->get_sdb();
    //qDebug()<<json_object;
    QVector<double>common_waves =
            jsn::getVectorDoubleFromJsonArray(json_object.value("_common_wave_grid").toArray());
    QJsonObject satellites = json_object.value("satellites").toObject();
    QJsonArray responses = satellites.value("landsat8").toObject().value("responses").toArray();
    auto landsat8_responses = jsn::getMatrixFromJsonArray(responses);
    QVector<double>respose1;
    QVector<double>respose2;
    QVector<double>respose3;
    QVector<double>respose4;
    QVector<double>respose5;
    for(int i=0;i<landsat8_responses.size();++i){
    respose1.append(landsat8_responses[i][0]);
    respose2.append(landsat8_responses[i][1]);
    respose3.append(landsat8_responses[i][2]);
    respose4.append(landsat8_responses[i][3]);
    respose5.append(landsat8_responses[i][4]);
    }
    dv::holdOn();
    dv::show(common_waves,respose1);
    dv::show(common_waves,respose2);
    dv::show(common_waves,respose3);
    dv::show(common_waves,respose4);
    dv::show(common_waves,respose5);
    dv::holdOff();
}

QTEST_MAIN(SatelliteComparatorUnitTests)
