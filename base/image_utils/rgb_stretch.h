#ifndef RGB_STRETCH_H
#define RGB_STRETCH_H
#include <QImage>

#include "satellites_structs.h"

//!
//! \brief buildSentinelRgbPercentile
//! Строит QImage RGB с percentile contrast stretch из трёх каналов.
//! Работает напрямую с uint16 DN — без перевода в КСЯ или яркость.
//! Универсально для Sentinel-2, Landsat и любого другого uint16 источника.
//!
//! \param red, green, blue — каналы R/G/B (sad::BAND_DATA с data=uint16*)
//! \param cloudMask — маска, 0=невалидный пиксель (nullptr = нет маски)
//! \param lowPct    — нижний процентиль [0..1], по умолчанию 0.02
//! \param highPct   — верхний процентиль [0..1], по умолчанию 0.98
//! \param gamma     — гамма-коррекция, по умолчанию 1.15
//!
QImage buildSentinelRgbPercentile(const sad::BAND_DATA& red,
                                  const sad::BAND_DATA& green,
                                  const sad::BAND_DATA& blue,
                                  const uint8_t* cloudMask = nullptr,
                                  double lowPct = 0.02, double highPct = 0.98,
                                  double gamma = 1.15);

#endif  // RGB_STRETCH_H
