#ifndef SLIDERSOFIMAGECORRECTORUNITTESTS_H
#define SLIDERSOFIMAGECORRECTORUNITTESTS_H

#include <QObject>
#include <QtTest>
#include <QSlider>

class SlidersOfImageCorrector;

class SlidersOfImageCorrectorUnitTests : public QObject
{
    Q_OBJECT

public:
    SlidersOfImageCorrectorUnitTests();

private:
    void helper_for_test_coef(const int slider_value,
                              const double expected_multiplier_value,
                              const QString& name_test,
                              SlidersOfImageCorrector *sic,
                              const uint header_width);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();


    void testSliderImageCorrector();

};

#endif // SLIDERSOFIMAGECORRECTORUNITTESTS_H
