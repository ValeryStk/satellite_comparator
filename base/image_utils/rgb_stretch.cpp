// sentinel_rgb_stretch.cpp
#include "rgb_stretch.h"

#include <QImage>
#include <QtGlobal>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

// Ищет PLO/PHI по накопленной гистограмме сырых uint16 DN
static bool computePercentileLimitsDN(const uint16_t* data, const uint8_t* mask,
                                      int width, int height, double lowPct,
                                      double highPct, uint16_t& plo,
                                      uint16_t& phi) {
    if (!data || width <= 0 || height <= 0) return false;

    const int total = width * height;

    // Находим реальный максимум в массиве за первый проход
    uint16_t maxVal = 0;
    for (int i = 0; i < total; ++i) {
        if (mask && mask[i] == 0) continue;
        if (data[i] > maxVal) maxVal = data[i];
    }
    if (maxVal == 0) return false;

    // Аллоцируем ровно столько бинов, сколько нужно
    const int BINS = int(maxVal) + 1;
    std::vector<uint32_t> hist(BINS, 0u);

    uint64_t count = 0;
    for (int i = 0; i < total; ++i) {
        if (mask && mask[i] == 0) continue;
        hist[data[i]]++;
        count++;
    }

    if (count == 0) return false;

    const uint64_t lowTarget = uint64_t(std::floor(lowPct * double(count - 1)));
    const uint64_t highTarget =
        uint64_t(std::floor(highPct * double(count - 1)));

    uint64_t cum = 0;
    int lowBin = 0, highBin = BINS - 1;
    bool lowFound = false;

    for (int k = 0; k < BINS; ++k) {
        cum += hist[k];
        if (!lowFound && cum > lowTarget) {
            lowBin = k;
            lowFound = true;
        }
        if (cum > highTarget) {
            highBin = k;
            break;
        }
    }

    plo = uint16_t(lowBin);
    phi = uint16_t(highBin + 1 < BINS ? highBin + 1 : BINS - 1);
    if (phi <= plo) phi = plo + 1;
    return true;
}

static inline uchar stretchToByte(uint16_t dn, uint16_t plo, uint16_t phi,
                                  double gammaInv) {
    double y;
    if (dn <= plo)
        y = 0.0;
    else if (dn >= phi)
        y = 1.0;
    else
        y = double(dn - plo) / double(phi - plo);

    if (gammaInv > 0.0) y = std::pow(y, gammaInv);

    return uchar(qBound(0, int(y * 255.0 + 0.5), 255));
}

}  // anonymous namespace

QImage buildSentinelRgbPercentile(const sad::BAND_DATA& red,
                                  const sad::BAND_DATA& green,
                                  const sad::BAND_DATA& blue,
                                  const uint8_t* cloudMask, double lowPct,
                                  double highPct, double gamma) {
    if (!red.data || !green.data || !blue.data) return QImage();
    if (red.width <= 0 || red.height <= 0) return QImage();

    const int W = red.width;
    const int H = red.height;

    uint16_t ploR, phiR, ploG, phiG, ploB, phiB;

    if (!computePercentileLimitsDN(red.data, cloudMask, W, H, lowPct, highPct,
                                   ploR, phiR) ||
        !computePercentileLimitsDN(green.data, cloudMask, W, H, lowPct, highPct,
                                   ploG, phiG) ||
        !computePercentileLimitsDN(blue.data, cloudMask, W, H, lowPct, highPct,
                                   ploB, phiB))
        return QImage();

    QImage img(W, H, QImage::Format_ARGB32);
    img.fill(Qt::black);

    const double gammaInv = (gamma > 0.0) ? (1.0 / gamma) : 1.0;
    auto* pixels = reinterpret_cast<QRgb*>(img.bits());
    const int total = W * H;

    for (int i = 0; i < total; ++i) {
        const uchar alpha = (!cloudMask || cloudMask[i] != 0) ? 255 : 0;
        pixels[i] =
            qRgba(stretchToByte(red.data[i], ploR, phiR, gammaInv),
                  stretchToByte(green.data[i], ploG, phiG, gammaInv),
                  stretchToByte(blue.data[i], ploB, phiB, gammaInv), alpha);
    }

    return img;
}
