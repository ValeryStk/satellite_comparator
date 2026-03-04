#include "sam.h"

#include <QVector>
#include <cmath>

#include "satellites_bands_map.h"

namespace sam {
// clang-format off
extern const char kSpectralIndexNDVI[] = "NDVI";
extern const char kSpectralIndexSWVI[] = "SWVI";
extern const char kSpectralIndexDSWI[] = "DSWI";
extern const char kSpectralIndexEVI[]  = "EVI";
// clang-format on

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

} // end namespace detail

ProcessingResult calculateEuclideanDistance(const QVector<double> &v1,
                                            const QVector<double> &v2,
                                            double &result) {
  ProcessingResult pr;
  detail::euclideanDistanceImpl(v1, v2, result, pr);
  return pr;
}

ProcessingResult calculateEuclideanDistance(const std::vector<double> &v1,
                                            const std::vector<double> &v2,
                                            double &result) {
  ProcessingResult pr;
  detail::euclideanDistanceImpl(v1, v2, result, pr);
  return pr;
}

double calculateNDVI(const double nir1, const double red) {
  return detail::calculate_normalized_difference(nir1, red);
}

double calculateSWVI(const double nir1, const double swir1) {
  return detail::calculate_normalized_difference(nir1, swir1);
}

double calculateDSWI(const double nir1, const double green, const double swir1,
                     const double red) {
  return (nir1 - green) / (swir1 + red);
}

double calculateEVI(const double nir1, double red, const double blue) {
  static const double G = 2.5;
  static const double C1 = 6.0;
  static const double C2 = 7.5;
  static const double L = 1;

  return G * (nir1 - red) / (nir1 + (C1 * red - C2 * blue) + L);
}

BandIndices getBandsIndexes(sk satellite) {
  BandIndices indices;
  indices.nir1 = getBandIndex(sam::kNIR1, satellite);
  indices.red = getBandIndex(sam::kRED, satellite);
  indices.green = getBandIndex(sam::kGREEN, satellite);
  indices.blue = getBandIndex(sam::kBLUE, satellite);
  indices.swir1 = getBandIndex(sam::kSWIR1, satellite);
  return indices;
}

} // end namespace sam
