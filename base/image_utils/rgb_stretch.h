#ifndef RGB_STRETCH_H
#define RGB_STRETCH_H

#include <QImage>

#include "satellites_structs.h"

// --- Основная функция: работает с сырыми uint16* ---
// Универсальная, подходит и для Sentinel, и для Landsat
QImage buildRgbPercentile(
    const uint16_t* red, const uint16_t* green, const uint16_t* blue, int width,
    int height,
    const uint16_t* cloudMask = nullptr,  // 0 = невалидный пиксель
    double lowPct = 0.02, double highPct = 0.98, double gamma = 1.15);

// --- Удобная обёртка для Sentinel (sad::BAND_DATA) ---
QImage buildSentinelRgbPercentile(const sad::BAND_DATA& red,
                                  const sad::BAND_DATA& green,
                                  const sad::BAND_DATA& blue,
                                  const uint16_t* cloudMask = nullptr,
                                  double lowPct = 0.02, double highPct = 0.98,
                                  double gamma = 1.15);

#endif  // RGB_STRETCH_H
