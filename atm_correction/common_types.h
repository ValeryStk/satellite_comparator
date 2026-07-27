#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "qmetatype.h"

struct result_values {
    double X;
    double q;
    double p;
    double h2O_power;
    double tau_0_a;
    double beta;
    double tau_e;
    double g;
    double albedo_1;
    double albedo_2;

    double err_X;
    double err_q;
    double err_p;
    double err_tau_mu_0;
    double err_tau_a0;
    double err_beta;
    double err_tau_e;
    double err_g;
    double err_albedo_1;
    double err_albedo_2;

    std::vector<double> fitted_speya;
};
Q_DECLARE_METATYPE(result_values)
#endif  // COMMON_TYPES_H
