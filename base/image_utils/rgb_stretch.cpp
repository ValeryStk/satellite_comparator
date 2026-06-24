// sentinel_rgb_stretch.cpp
#include "rgb_stretch.h"

#include <QImage>
#include <QtGlobal>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

// Конвертирует uint16 DN -> КСЯ (float) через linear scale
static inline float dnToReflectance(uint16_t dn, double mult, double add) {
    float r = float(double(dn) * mult + add);
    return (r < 0.0f) ? 0.0f : r;
}

// Строит гистограмму КСЯ и находит percentile границы
static bool computePercentileLimits(const uint16_t* data, const uint8_t* mask,
                                    int width, int height, double mult,
                                    double add, float cmin, float cmax,
                                    double lowPct, double highPct, int bins,
                                    float& plo, float& phi) {
    if (!data || width <= 0 || height <= 0 || cmax <= cmin) return false;

    std::vector<uint32_t> hist(bins, 0u);
    const float delta = (cmax - cmin) / float(bins);
    const int total = width * height;
    uint64_t count = 0;

    for (int i = 0; i < total; ++i) {
        if (mask && mask[i] == 0) continue;

        float x = dnToReflectance(data[i], mult, add);
        if (x < cmin) x = cmin;
        if (x > cmax) x = cmax;

        int k = int((x - cmin) / delta);
        if (k < 0) k = 0;
        if (k >= bins) k = bins - 1;
        hist[k]++;
        count++;
    }

    if (count == 0) return false;

    const uint64_t lowTarget = uint64_t(std::floor(lowPct * double(count - 1)));
    const uint64_t highTarget =
        uint64_t(std::floor(highPct * double(count - 1)));

    uint64_t cum = 0;
    int lowBin = 0, highBin = bins - 1;
    bool lowFound = false;

    for (int k = 0; k < bins; ++k) {
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

    plo = cmin + float(lowBin) * delta;
    phi = cmin + float(highBin + 1) * delta;
    if (phi <= plo) phi = plo + delta;
    return true;
}

static inline uchar stretchToByte(float x, float plo, float phi,
                                  double gammaInv) {
    double y;
    if (x <= plo)
        y = 0.0;
    else if (x >= phi)
        y = 1.0;
    else
        y = double(x - plo) / double(phi - plo);

    if (gammaInv > 0.0) y = std::pow(y, gammaInv);

    int v = int(y * 255.0 + 0.5);
    return uchar(qBound(0, v, 255));
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

    // Диапазон КСЯ для Sentinel-2: типично 0.0 .. 1.0
    // Берём с небольшим запасом до 1.5 на случай засветок
    constexpr float CMIN = 0.0f;
    constexpr float CMAX = 1.5f;
    constexpr int BINS = 4096;

    float ploR, phiR, ploG, phiG, ploB, phiB;

    if (!computePercentileLimits(red.data, cloudMask, W, H,
                                 red.reflectance_mult, red.reflectance_add,
                                 CMIN, CMAX, lowPct, highPct, BINS, ploR, phiR))
        return QImage();
    if (!computePercentileLimits(green.data, cloudMask, W, H,
                                 green.reflectance_mult, green.reflectance_add,
                                 CMIN, CMAX, lowPct, highPct, BINS, ploG, phiG))
        return QImage();
    if (!computePercentileLimits(blue.data, cloudMask, W, H,
                                 blue.reflectance_mult, blue.reflectance_add,
                                 CMIN, CMAX, lowPct, highPct, BINS, ploB, phiB))
        return QImage();

    QImage img(W, H, QImage::Format_ARGB32);
    img.fill(Qt::black);

    const double gammaInv = (gamma > 0.0) ? (1.0 / gamma) : 1.0;
    const int total = W * H;

    auto* pixels = reinterpret_cast<QRgb*>(img.bits());

    for (int i = 0; i < total; ++i) {
        uchar alpha = 255;
        if (cloudMask && cloudMask[i] == 0) alpha = 0;

        float r = dnToReflectance(red.data[i], red.reflectance_mult,
                                  red.reflectance_add);
        float g = dnToReflectance(green.data[i], green.reflectance_mult,
                                  green.reflectance_add);
        float b = dnToReflectance(blue.data[i], blue.reflectance_mult,
                                  blue.reflectance_add);

        pixels[i] = qRgba(stretchToByte(r, ploR, phiR, gammaInv),
                          stretchToByte(g, ploG, phiG, gammaInv),
                          stretchToByte(b, ploB, phiB, gammaInv), alpha);
    }

    return img;
}
