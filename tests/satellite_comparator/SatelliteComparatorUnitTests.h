#ifndef SATELLITECOMPARATORUNITTESTS_H
#define SATELLITECOMPARATORUNITTESTS_H

#include <QObject>
#include <QtTest>


class SatelliteComparatorUnitTests : public QObject
{
    Q_OBJECT

public:
    SatelliteComparatorUnitTests();


private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testSatelliteComparatorBaseCheck();
    void testSatelliteComparatorLinearInterpolation();

};

#endif // SATELLITECOMPARATORUNITTESTS_H
