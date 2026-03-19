#ifndef UNITTESTS_H
#define UNITTESTS_H

#include <QObject>
#include <QtTest>

class atm_correction_UnitTests : public QObject {
    Q_OBJECT

public:
    atm_correction_UnitTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
};

#endif  // UNITTESTS_H
