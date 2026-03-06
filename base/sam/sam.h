#ifndef SAM_H
#define SAM_H

#include <QVector>
#include <vector>

#include "satellites_bands_map.h"

// spectral analyzing module (sam)
namespace sam {

// clang-format off
struct BandIndices {
    int red   = -1;
    int green = -1;
    int blue  = -1;
    int nir1  = -1;
    int swir1 = -1;
    int swir2 = -1;
};
// clang-format on

BandIndices getBandsIndexes(sam::sk satellite);

enum class STATUS_CODE {
    OK = 0,
    SIZES_ARE_NOT_THE_SAME,
    ONE_OF_THE_VECTORS_ARE_EMPTY,
    UNEXPECTED_RESULT
};

struct ProcessingResult {
    STATUS_CODE status;
    std::string message;
};

ProcessingResult calculateEuclideanDistance(const std::vector<double> &v1,
                                            const std::vector<double> &v2,
                                            double &result);

ProcessingResult calculateEuclideanDistance(const QVector<double> &v1,
                                            const QVector<double> &v2,
                                            double &result);

inline double calculateNDVI(const double nir1, const double red);

inline double calculateSWVI(const double nir1, const double swir1);

inline double calculateDSWI(const double nir1, const double green,
                            const double swir1, const double red);

inline double calculateEVI(const double nir1, const double red,
                           const double blue);

inline double calculateNBR(const double nir1, const double swir2);

inline double calculateNDSWIR(const double nir1, const double swir1);

inline double calculateNBRSWIR(const double swir2, const double swir1);

}  // end namespace sam

#endif  // SAM_H
