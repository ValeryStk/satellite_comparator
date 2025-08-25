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

inline ProcessingResult euclideanDistance(const vector<double> &v1,
                                const vector<double> &v2,
                                double& result);

inline ProcessingResult euclideanDistance(const QVector<double> &v1,
                                const QVector<double> &v2,
                                double& result);

}

#endif // SAM_H
