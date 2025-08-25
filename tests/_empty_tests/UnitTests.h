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
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

};

#endif // UNITTESTS_H
