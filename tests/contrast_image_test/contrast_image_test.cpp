
#include "contrast_image_test.h"

#include <QDebug>
#include <QDesktopServices>
#include <QImage>
#include <QUrl>

namespace {
static void applyContrast(QImage &img, double contrast) {
    // contrast: 0.0 — без изменений, >0 — больше контраст, <0 — меньше
    // разумные значения: от -0.5 до 0.5

    img = img.convertToFormat(QImage::Format_ARGB32);

    int width = img.width();
    int height = img.height();

    // подготовим LUT (таблицу преобразования 0..255)
    uchar lut[256];
    for (int v = 0; v < 256; ++v) {
        // нормализуем в [0,1]
        double x = v / 255.0;

        // S‑образная кривая (по типу «Curves»)
        // можно поиграть формулой, это один из вариантов
        double y = x - 0.5;
        y = y * (1.0 + contrast);  // растягиваем вокруг середины
        y = qBound(-0.5, y, 0.5);
        y += 0.5;

        int out = int(y * 255.0 + 0.5);
        lut[v] = uchar(qBound(0, out, 255));
    }

    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb p = line[x];

            int r = lut[qRed(p)];
            int g = lut[qGreen(p)];
            int b = lut[qBlue(p)];

            line[x] = qRgba(r, g, b, qAlpha(p));
        }
    }
}
}  // end namespace

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
        applyContrast(img2, 1.5);
        img2.save(QCoreApplication::applicationDirPath() +
                  "/contrasted_change_detection.png");
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(QCoreApplication::applicationDirPath() +
                                "/last_change_detection.png"));

        QDesktopServices::openUrl(
            QUrl::fromLocalFile(QCoreApplication::applicationDirPath() +
                                "/contrasted_change_detection.png"));
    } else {
        qWarning("Failed to load image");
    }
}

QTEST_MAIN(contrast_image_test)
