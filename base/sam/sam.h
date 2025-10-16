#ifndef SAM_H
#define SAM_H

#include <vector>
#include <QVector>

namespace sam {

using std::vector;

enum class STATUS_CODE {
    OK = 0,
    SIZES_ARE_NOT_THE_SAME,
    ONE_OF_THE_VECTORS_ARE_EMPTY,
    UNEXPECTED_RESULT
};

struct ProcessingResult {
  sam::STATUS_CODE status;
  std::string message;
};

inline ProcessingResult calculateEuclideanDistance(const vector<double> &v1,
                                const vector<double> &v2,
                                double& result);

inline ProcessingResult calculateEuclideanDistance(const QVector<double> &v1,
                                const QVector<double> &v2,
                                double& result);

//Нормализованный индекс растительности NDVI (Оценка плотности и состояния растительного покрова)
inline double calculateNDVI(const double NIR_value,
                            const double Red_value);

//Нормализованный индекс водного содержания NDWI (Определение водных объектов и влажности растительности)
inline double calculateNDWI(const double NIR_value,
                            const double SWIR1_value);
}

#endif // SAM_H
