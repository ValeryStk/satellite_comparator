
#ifndef CONTRAST_IMAGE_TEST_H
#define CONTRAST_IMAGE_TEST_H

#include <QObject>
#include <QtTest>

class contrast_image_test : public QObject {
    Q_OBJECT

public:
    contrast_image_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void contrastImage();
};

#endif  // CONTRAST_IMAGE_TEST_H
