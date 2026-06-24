#ifndef RGB_STRETCH_H
#define RGB_STRETCH_H

#include <QImage>
#include <QVector>

#include "satellites_structs.h"

//!
//! \brief buildSentinelRgbPercentile
//! Строит QImage RGB с percentile contrast stretch из трёх каналов Sentinel-2.
//! Принимает BAND_DATA с uint16 данными (сырые DN),
//! автоматически конвертирует в КСЯ через reflectance_mult/reflectance_add.
//!
//! \param red   — канал для R (обычно B4, ~665нм)
//! \param green — канал для G (обычно B3, ~560нм)
//! \param blue  — канал для B (обычно B2, ~490нм)
//! \param cloudMask — маска облаков uint8, 0=невалидный пиксель (nullptr = нет
//! маски) \param lowPct  — нижний процентиль [0..1], по умолчанию 0.02 \param
//! highPct — верхний процентиль [0..1], по умолчанию 0.98 \param gamma   —
//! гамма-коррекция, по умолчанию 1.15 \return готовый QImage Format_ARGB32
//!
QImage buildSentinelRgbPercentile(const sad::BAND_DATA &red,
                                  const sad::BAND_DATA &green,
                                  const sad::BAND_DATA &blue,
                                  const uint8_t *cloudMask = nullptr,
                                  double lowPct = 0.02, double highPct = 0.98,
                                  double gamma = 1.15);

#endif  // RGB_STRETCH_H
