#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "qmetatype.h"

struct result_values {
    double tau_0_a;
    double beta;
    double g;
    double albedo;
    double err_tau;
    double err_beta;
    double err_g;
    double err_albedo;
};
Q_DECLARE_METATYPE(result_values)
#endif  // COMMON_TYPES_H
