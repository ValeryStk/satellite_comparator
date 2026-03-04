#ifndef UNITTESTS_H
#define UNITTESTS_H

#include <QObject>
#include <QtTest>

class message_reporter_UnitTests : public QObject {
    Q_OBJECT

public:
    message_reporter_UnitTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void showMessages();
};

#endif  // UNITTESTS_H
