
#include "contrast_image_test.h"

#include <QDebug>

#include "image_utils.h"

contrast_image_test::contrast_image_test() {}

void contrast_image_test::initTestCase() {
    // Инициализация перед запуском всех тестов
}

void contrast_image_test::cleanupTestCase() {
    // Очистка после выполнения всех тестов
}

void contrast_image_test::init() {
    // Инициализация перед каждым тестом
}

void contrast_image_test::cleanup() {
    // Очистка после каждого теста
}

void contrast_image_test::contrastImage() {
    QImage img;
    QImage img2;
    if (img.load(QCoreApplication::applicationDirPath() +
                 "/last_change_detection.png")) {
        img2 = img;
        applyContrast(img2);
        img2.save(QCoreApplication::applicationDirPath() +
                  "/contrasted_change_detection.png");
        openImageByDesktop("last_change_detection.png");
        openImageByDesktop("contrasted_change_detection.png");

    } else {
        qWarning("Failed to load image");
    }
}

QTEST_MAIN(contrast_image_test)
