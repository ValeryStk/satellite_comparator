#ifndef UNITTESTS_H
#define UNITTESTS_H

#include <QObject>
#include <QtTest>

class bekas_UnitTests : public QObject
{
    Q_OBJECT

public:
    bekas_UnitTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void bekasTest();

};

#endif // UNITTESTS_H
