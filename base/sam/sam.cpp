#include "sam.h"

#include <qmath.h>

#include <QVector>

#include "satellites_bands_map.h"

namespace sam {

namespace detail {

template <typename Container>
ProcessingResult euclideanDistanceImpl(const Container &v1, const Container &v2,
                                       double &result, ProcessingResult &pr) {
    result = 0;
    pr.message = "unexpected result";
    pr.status = STATUS_CODE::UNEXPECTED_RESULT;

    if (v1.size() == 0 || v2.size() == 0) {
        pr.message = "one of the vectors is empty";
        pr.status = STATUS_CODE::ONE_OF_THE_VECTORS_ARE_EMPTY;
        return pr;
    }
    if (v1.size() != v2.size()) {
        pr.message = "Vectors sizes are not the same";
        pr.status = STATUS_CODE::SIZES_ARE_NOT_THE_SAME;
        return pr;
    }
    double sum = 0.0;
    for (int i = 0; i < static_cast<int>(v1.size()); ++i) {
        double diff = v1[i] - v2[i];
        sum += diff * diff;
    }
    result = std::sqrt(sum);
    pr.message = "OK";
    pr.status = STATUS_CODE::OK;
    return pr;
}

double calculate_normalized_difference(const double a, const double b) {
    double sum = a + b;
    return std::abs(sum) < 1e-10 ? 0.0 : (a - b) / sum;
}

}  // end namespace detail

// clang-format off
BandIndicesIndexes getBandsIndexes(sk satellite) {
    BandIndicesIndexes indices;
    indices.aer   = getBandIndex(sam::kAER,   satellite); //AER    1
    indices.blue  = getBandIndex(sam::kBLUE,  satellite); //BLUE   2
    indices.green = getBandIndex(sam::kGREEN, satellite); //GREEN  3
    indices.red   = getBandIndex(sam::kRED,   satellite); //RED    4
    indices.re1   = getBandIndex(sam::kRE1,   satellite); //RE1    5
    indices.re2   = getBandIndex(sam::kRE2,   satellite); //RE2    6
    indices.re3   = getBandIndex(sam::kRE3,   satellite); //RE3    7
    indices.nir1  = getBandIndex(sam::kNIR1,  satellite); //NIR1   8
    indices.nir2  = getBandIndex(sam::kNIR2,  satellite); //NIR2   9
    indices.wv    = getBandIndex(sam::kWV,    satellite); //WV     10
    indices.swir1 = getBandIndex(sam::kSWIR1, satellite); //SWIR1  11
    indices.swir2 = getBandIndex(sam::kSWIR2, satellite); //SWIR2  12
    indices.swir3 = getBandIndex(sam::kSWIR3, satellite); //SWIR3  13
    return indices;
}
// clang-format on
ProcessingResult calculateEuclideanDistance(const QVector<double> &v1,
                                            const QVector<double> &v2) {
    ProcessingResult pr;
    detail::euclideanDistanceImpl(v1, v2, pr.value, pr);
    return pr;
}

ProcessingResult calculateEuclideanDistance(const std::vector<double> &v1,
                                            const std::vector<double> &v2) {
    ProcessingResult pr;
    detail::euclideanDistanceImpl(v1, v2, pr.value, pr);
    return pr;
}

double calculateNDVI(const double nir1, const double red) {
    return detail::calculate_normalized_difference(nir1, red);
}

double calculateSWVI(const double nir1, const double swir1) {
    return detail::calculate_normalized_difference(nir1, swir1);
}

double calculateDSWI(const double nir1, const double green, const double swir2,
                     const double red) {
    return (nir1 - green) / (swir2 + red);
}

double calculateEVI(const double nir1, double red, const double blue) {
    static const double G = 2.5;
    static const double C1 = 6.0;
    static const double C2 = 7.5;
    static const double L = 1;

    return G * (nir1 - red) / (nir1 + (C1 * red - C2 * blue) + L);
}

double calculateNBR(const double nir2, const double swir3) {
    return detail::calculate_normalized_difference(nir2, swir3);
}

double calculateNDSWIR(const double nir2, const double swir2) {
    return detail::calculate_normalized_difference(nir2, swir2);
}

double calculateNBRSWIR(const double swir3, const double swir2) {
    return (swir3 - swir2 - 0.02) / (swir3 + swir2 + 0.1);
}

double calculateSpeyaFromSentinelDN(const double DN,  // DN WITH OFFSET
                                    const double solarIrradiance,
                                    const double cosSunZenitAngle) {
    return (DN * solarIrradiance * cosSunZenitAngle) / (M_PI * 10000);
}

double calculateSpeyaFromSentinelKsy(const double Ksy,
                                     const double solarIrradiance,
                                     const double cosSunZenitAngle) {
    return (Ksy * solarIrradiance * cosSunZenitAngle) / M_PI;
}

}  // end namespace sam
