#include "image_utils.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QtGlobal>

void applyContrast(QImage &img, double contrast) {
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
void openImageByDesktop(const QString &imgName) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(
        QCoreApplication::applicationDirPath() + "/" + imgName));
};
