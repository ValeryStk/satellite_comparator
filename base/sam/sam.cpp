#include "sam.h"

#include <cmath>
#include <QDebug>


namespace sam {

namespace detail {

template <typename Container>
inline ProcessingResult euclideanDistanceImpl(const Container& v1,
                                              const Container& v2,
                                              double& result,
                                              ProcessingResult &pr)
{

    result = 0;
    pr.message = "unexpected result";
    pr.status = STATUS_CODE::UNEXPECTED_RESULT;

    if(v1.size()==0||v2.size()==0){
        pr.message = "one of the vectors is empty";
        pr.status = STATUS_CODE::ONE_OF_THE_VECTORS_ARE_EMPTY;
        return pr;
    }
    if (v1.size() != v2.size()) {
        pr.message = "Vectors sizes are not the same";
        pr.status = STATUS_CODE::SIZES_ARE_NOT_THE_SAME;
        return  pr;
    }
    double sum = 0.0;
    for (int i = 0; i < static_cast<int>(v1.size()); ++i) {
        sum += std::pow(v1[i] - v2[i], 2);
    }
    result = std::sqrt(sum);
    pr.message = "OK";
    pr.status = STATUS_CODE::OK;
    return pr;
}

inline double calculate_normolized_difference(const double a,
                                              const double b){
 return (a - b) / (a + b);
}

} //end namespace detail


inline ProcessingResult calculateEuclideanDistance(const QVector<double> &v1,
                                                   const QVector<double> &v2,
                                                   double &result)
{

    ProcessingResult pr;
    detail::euclideanDistanceImpl(v1, v2, result, pr);
    return pr;
}

inline ProcessingResult calculateEuclideanDistance(const vector<double> &v1,
                                                   const vector<double> &v2,
                                                   double &result
                                                   )
{
    ProcessingResult pr;
    detail::euclideanDistanceImpl(v1, v2, result, pr);
    return pr;
}

double calculateNDVI(const double NIR_value,
                     const double Red_value)
{
    return detail::calculate_normolized_difference(NIR_value, Red_value);
}

double calculateNDWI(const double NIR_value,
                     const double SWIR1_value)
{
    return detail::calculate_normolized_difference(NIR_value, SWIR1_value);
}

} //end namespace sam
