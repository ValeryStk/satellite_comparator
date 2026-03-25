#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <QImage>

void applyContrast(QImage& img, double contrast = 0.5);

void openImageByDesktop(const QString& imgName);

#endif  // IMAGE_UTILS_H
