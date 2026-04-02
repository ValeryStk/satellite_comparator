#ifndef SAM_H
#define SAM_H

#include <QVector>
#include <vector>

#include "satellites_bands_map.h"

// spectral analyzing module (sam)
namespace sam {

// clang-format off
struct BandIndicesIndexes {
    int aer   = -1; //AER    1
    int blue  = -1; //BLUE   2
    int green = -1; //GREEN  3
    int red   = -1; //RED    4
    int re1   = -1; //RE1    5
    int re2   = -1; //RE2    6
    int re3   = -1; //RE3    7
    int nir1  = -1; //NIR1   8
    int nir2  = -1; //NIR2   9
    int wv    = -1; //WV     10
    int swir1 = -1; //SWIR1  11
    int swir2 = -1; //SWIR2  12
    int swir3 = -1; //SWIR3  13
};

struct BandIndicesValues {
    double aer   = NAN; //AER    1
    double blue  = NAN; //BLUE   2
    double green = NAN; //GREEN  3
    double red   = NAN; //RED    4
    double re1   = NAN; //RE1    5
    double re2   = NAN; //RE2    6
    double re3   = NAN; //RE3    7
    double nir1  = NAN; //NIR1   8
    double nir2  = NAN; //NIR2   9
    double wv    = NAN; //WV     10
    double swir1 = NAN; //SWIR1  11
    double swir2 = NAN; //SWIR2  12
    double swir3 = NAN; //SWIR3  13
};

// clang-format on

BandIndicesIndexes getBandsIndexes(sam::sk satellite);

enum class STATUS_CODE {
    OK = 0,
    SIZES_ARE_NOT_THE_SAME,
    ONE_OF_THE_VECTORS_ARE_EMPTY,
    UNEXPECTED_RESULT
};

struct ProcessingResult {
    STATUS_CODE status;
    std::string message;
    double value;
};

ProcessingResult calculateEuclideanDistance(const std::vector<double> &v1,
                                            const std::vector<double> &v2);

ProcessingResult calculateEuclideanDistance(const QVector<double> &v1,
                                            const QVector<double> &v2);

inline double calculateNDVI(const double nir1, const double red);

inline double calculateSWVI(const double nir1, const double swir1);

inline double calculateDSWI(const double nir1, const double green,
                            const double swir2, const double red);

inline double calculateEVI(const double nir1, const double red,
                           const double blue);

inline double calculateNBR(const double nir2, const double swir3);

inline double calculateNDSWIR(const double nir2, const double swir2);

inline double calculateNBRSWIR(const double swir3, const double swir2);

inline double calculateSpeyaFromSentinelDN(const double DN,
                                           const double solarIrradiance,
                                           const double cosSunZenitAngle);

inline double calculateSpeyaFromSentinelKsy(const double Ksy,
                                            const double solarIrradiance,
                                            const double cosSunZenitAngle);

}  // end namespace sam

#endif  // SAM_H
