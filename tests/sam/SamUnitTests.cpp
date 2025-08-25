#include "SamUnitTests.h"

#include <QDebug>
#include "../../base/sam/sam.cpp"


namespace{


} // end namespace


SamUnitTests::SamUnitTests()
{
}

void SamUnitTests::initTestCase()
{
    // Инициализация перед запуском всех тестов
}

void SamUnitTests::cleanupTestCase()
{
    // Очистка после выполнения всех тестов
}

void SamUnitTests::init()
{
    // Инициализация перед каждым тестом

}

void SamUnitTests::cleanup()
{
    // Очистка после каждого теста
}

void SamUnitTests::samMetricsTest()
{
    double result;
    sam::ProcessingResult pr;
    QVector<double> v1 = {};
    QVector<double> v2 = {};
    pr = sam::euclideanDistance(v1,v2,result);
    QString message = QString::fromStdString(pr.message);
    QCOMPARE(pr.status,sam::STATUS_CODE::ONE_OF_THE_VECTORS_ARE_EMPTY);
    qDebug()<<message<<result;
    v1 = {0.8,0.6,0.2};
    v2 = {0.8,0.55,0.25};
    pr = sam::euclideanDistance(v1,v2,result);
    QCOMPARE(pr.status,sam::STATUS_CODE::OK);
    qDebug()<<QString::fromStdString(pr.message)<<result;
}



QTEST_MAIN(SamUnitTests)
