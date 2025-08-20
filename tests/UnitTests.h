#ifndef UNITTESTS_H
#define UNITTESTS_H

#include <QObject>
#include <QtTest>
#include <QSlider>

class UnitTests : public QObject
{
    Q_OBJECT

public:
    UnitTests();

private:
    void helper_for_test_coef(int slider_value,
                              double multiplier_value,
                              const char* name_test);

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
