#ifndef UNITTESTS_H
#define UNITTESTS_H

#include <QObject>
#include <QtTest>

class MPFIT_wrapper_UnitTests : public QObject
{
    Q_OBJECT

public:
    MPFIT_wrapper_UnitTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

};

#endif // UNITTESTS_H
