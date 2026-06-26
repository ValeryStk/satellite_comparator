#ifndef RGB_STRETCH_H
#define RGB_STRETCH_H

#include <QImage>

// --- Основная функция: работает с сырыми uint16* ---
// Универсальная, подходит и для Sentinel, и для Landsat
QImage buildRgbPercentile(
    const uint16_t* red, const uint16_t* green, const uint16_t* blue, int width,
    int height,
    const uint16_t* cloudMask = nullptr,  // 0 = невалидный пиксель
    double lowPct = 0.02, double highPct = 0.98, double gamma = 1.15);

// Версия с готовыми границами (для единого растяжения по всему временному ряду)
QImage buildRgbPercentileFixed(const uint16_t* red, const uint16_t* green,
                               const uint16_t* blue, int width, int height,
                               uint16_t ploR, uint16_t phiR, uint16_t ploG,
                               uint16_t phiG, uint16_t ploB, uint16_t phiB,
                               double gamma = 1.15,
                               const uint16_t* cloudMask = nullptr);

// Утилита: вычислить перцентильные границы по нескольким снимкам сразу
bool computePercentileLimitsGlobal(const std::vector<const uint16_t*>& channels,
                                   int width, int height, double lowPct,
                                   double highPct, uint16_t& plo,
                                   uint16_t& phi);

#endif  // RGB_STRETCH_H
