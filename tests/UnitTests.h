#ifndef UNITTESTS_H
#define UNITTESTS_H

#include <QObject>
#include <QtTest>

class UnitTests : public QObject
{
    Q_OBJECT

public:
    UnitTests();

private slots:
    void initTestCase();     // Вызывается перед выполнением первого тестового метода
    void cleanupTestCase();  // Вызывается после выполнения последнего тестового метода
    void init();             // Вызывается перед каждым тестовым методом
    void cleanup();          // Вызывается после каждого тестового метода


    void testSliderImageCorrector();

    void testSatelliteComparatorBaseCheck();
    void testSatelliteComparatorLinearInterpolation();


};

#endif // UNITTESTS_H
