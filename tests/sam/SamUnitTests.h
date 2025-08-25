#ifndef SAMUNITTESTS_H
#define SAMUNITTESTS_H

#include <QObject>
#include <QtTest>

class SamUnitTests : public QObject
{
    Q_OBJECT

public:
    SamUnitTests();


private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void samMetricsTest();



};

#endif // SAMUNITTESTS_H
