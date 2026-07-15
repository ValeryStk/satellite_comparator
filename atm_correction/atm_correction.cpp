#include "atm_correction.h"

#include <stdio.h>
#include <stdlib.h>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QVector>
#include <QtMath>
#include <array>
#include <cmath>
#include <vector>

#include "common_types.h"
#include "davis.h"
#include "json_utils.h"
#include "math.h"
#include "mpfit.h"

std::vector<double> speya_result;
std::vector<double> e_result;
std::vector<double> B_atm_result;
std::vector<double> omega_lambda_list;
std::vector<double> tau_lambda_list;
std::vector<double> x_list;
std::vector<double> ro_0_list;
std::vector<double> B1_result;
std::vector<double> B2_result;
std::vector<double> B_result;

std::vector<double> final_B_result(10, 0);

std::vector<double> albedo_final_result;

std::vector<double> tau_a_res;
std::vector<double> tau_m_res;
double x_m_res;
double x_a_res;
double lambda_1 = 400;
double lambda_2 = 665;
// std::vector<double> B_sun_list;

/*
auto b_atm = omega_lambda[i] * B_sun_list[i] * x[i] /
             (4.0 * (mu + mu_0)) *
             (1.0 - exp(-tau_lambda[i] * (1.0 / mu_0 + 1.0 / mu))) *
             (1.0 + Q * pow(omega_lambda[i] * tau_lambda[i], P));
result.push_back(b_atm);*/

namespace lss {
void updateSatelliteResponses(const QString& satellite_name);
}

namespace {

inline std::vector<double> calculateAlbedoFinal(
    const QVector<double>& speya_values);

bool allEqual(const std::vector<double>& v, double eps = 1e-9) {
    if (v.empty())
        return true;  // или false, если пустой вектор не считается "одинаковым"
    double first = v[0];
    for (size_t i = 1; i < v.size(); ++i) {
        if (std::fabs(v[i] - first) > eps) {
            return false;
        }
    }
    return true;
}

using std::string;
using std::vector;

static bool is_first_run = true;
constexpr int NUMBER_OF_CHANNELS = 10;
constexpr uint16_t NUMBER_WAVELENGTH = 601;

constexpr double LAMBDA_0 = 550;
constexpr double pi = 3.14159265358979323846;

// double Q = 1;
// double P = 1.25;
// double TAU_M_0 = 0.101;
// double TAU_E = 0.04;

double mu_0 = 0.0;  // косинус зенитного угла солнца
double mu = 0.0;    // косинус зенитного угла съёмки
double fi = 0.0;  // косинус разности азимутальных углов съёмки и солнца
double gamma = 0.0;  // косинус угла рассеяния

constexpr double dobson_TiO[NUMBER_OF_CHANNELS] = {
    0.985, 0.992, 0.995, 0.996, 0.997, 0.997, 0.998, 0.999, 0.999, 0.997};
constexpr double dobson_alfa[NUMBER_OF_CHANNELS] = {
    0.0001,  0.00003, 0.00002,  0.00001,  0.00001,
    0.00001, 0.00001, 0.000005, 0.000005, 0.00002};

constexpr double central_waves[NUMBER_OF_CHANNELS] = {443, 492, 560, 664, 704,
                                                      740, 780, 840, 865, 945};

std::vector<double> v_central_waves = {443, 492, 560, 664, 704,
                                       740, 780, 840, 865, 945};

struct vars_struct {
    double* tau_0_a_err;
    double* beta_err;
    double* g_err;
    double* albedo_err;
};

static result_values rv;

inline double compute_mH2O(double B9, double B8a);

inline double compute_TO3(double TiO3, double alfa_i, double X);

inline void compute_TO3_list(double X);

inline double compute_gamma(double mu, double mu_0, double fi);

void calculDividerList(vector<vector<double>>& responses);

inline vector<double> compute_tau_m(const vector<double>& list, double tau_m_0);

inline double compute_ro_0(double ro_1, double ro_2, double lambda,
                           double lambda_1, double lambda_2);

void loadAllLists() {
    if (!is_first_run) return;
    is_first_run = false;

    jsn::getJsonArrayFromFile(":/atm_params/atm_params.json", atm_params);
    jsn::getJsonArrayFromFile(
        ":/responses/sentinel2A/sentinel2A_responses.json",
        sat_sentinel2A_respns);
    jsn::getJsonArrayFromFile(
        ":/responses/sentinel2B/sentinel2B_responses.json",
        sat_sentinel2B_respns);

    for (int i = 0; i < atm_params.size(); ++i) {
        // double h2o = atm_params[i].toObject()["h2o"].toDouble();
        // T_H2O_list.push_back(h2o);
        double o2 = atm_params[i].toObject()["o2"].toDouble();
        T_O2_list.push_back(o2);  // В новом методе не будет использоваться
        // double o3 = atm_params[i].toObject()["o3"].toDouble();
        // T_O3_list.push_back(o3); В новом методе расчитывается для центральной
        // длины волны
        //  и вынесен из под интеграла
        double wl = atm_params[i].toObject()["wavelength"].toDouble();
        lambda_list.push_back(wl);
        double sun = atm_params[i].toObject()["sun"].toDouble();
        B_lambda_teta_list.push_back(sun);
    }
    tau_m = compute_tau_m(lambda_list, 0.1);
    satellite_name_key = "";
    lss::updateSatelliteResponses("sentinel 2A");
    T_O3_list.resize(NUMBER_OF_CHANNELS);
    compute_TO3_list(300);
}

void calculDividerList(vector<vector<double>>& responses) {
    if (!divider_list.empty()) divider_list.clear();
    divider_list.resize(responses.size());
    size_t sizeList = responses[0].size();
    Q_ASSERT(sizeList == NUMBER_WAVELENGTH);
    for (uintmax_t i = 0; i < responses.size(); ++i) {
        for (size_t j = 0; j < sizeList; ++j) {
            divider_list[i] += responses[i][j];
        }
    }
    /*for (int i = 0; i < divider_list.size(); ++i) {
        //qDebug() << "DIVIDED: " << divider_list[i];
    }*/
}

inline vector<double> compute_tau_m(const vector<double>& list,
                                    double tau_m_0) {
    std::vector<double> result;
    for (uintmax_t i = 0; i < list.size(); ++i) {
        double lambda_0_lambda = LAMBDA_0 / list[i];
        result.push_back(tau_m_0 * pow(lambda_0_lambda, 4));
    }
    return result;
}

inline double compute_x_m(const double gamma) {
    return 3 * (1 + gamma * gamma) / 4;
}

inline double compute_x_a(const double g) {
    return ((1 - g * g) / pow((1 + g * g) - (2 * g * gamma), 1.5));
}

inline vector<double> compute_tau_a(const double& tau_0_a, const double& beta,
                                    const vector<double>& list) {
    std::vector<double> result;
    for (uintmax_t i = 0; i < list.size(); ++i) {
        double lambda_0_lambda = LAMBDA_0 / list[i];
        result.push_back(tau_0_a * pow(lambda_0_lambda, beta));
    }
    return result;
}

inline vector<double> compute_tau_lambda(const double& tau_e,
                                         const double& tau_0_a,
                                         const double& beta,
                                         const vector<double>& tau_m,
                                         const vector<double>& list) {
    std::vector<double> result;
    std::vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    for (uintmax_t i = 0; i < list.size(); ++i) {
        result.push_back(tau_m[i] + tau_a[i] + tau_e);
    }
    return result;
}

inline vector<double> compute_omega(const double& tau_e, const double& tau_0_a,
                                    const double& beta,
                                    const vector<double>& tau_m,
                                    const vector<double>& list) {
    std::vector<double> result;
    std::vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    for (uintmax_t i = 0; i < list.size(); ++i) {
        result.push_back((tau_m[i] + tau_a[i]) / (tau_m[i] + tau_a[i] + tau_e));
    }
    return result;
}

inline vector<double> compute_g(const double& g, const double& tau_0_a,
                                const double& beta, const vector<double>& tau_m,
                                const vector<double>& list) {
    vector<double> result;
    vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    for (uintmax_t i = 0; i < list.size(); ++i) {
        result.push_back(g * tau_a[i] / (tau_m[i] + tau_a[i]));
    }
    return result;
}

inline vector<double> compute_x(const double& mu_0, const double& g,
                                const double& tau_0_a, const double& beta,
                                const vector<double>& tau_m,
                                const vector<double>& list) {
    vector<double> result;
    vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    auto x_m = compute_x_m(gamma);
    auto x_a = compute_x_a(g);
    for (uintmax_t i = 0; i < list.size(); ++i) {
        double znamenatel = tau_m[i] + tau_a[i];
        result.push_back(x_m * tau_m[i] / znamenatel +
                         x_a * tau_a[i] / znamenatel);
    }
    tau_a_res = tau_a;
    tau_m_res = tau_m;
    x_m_res = x_m;
    x_a_res = x_a;
    /*if (allEqual(result)) {
        qDebug()
            << "****************** AL EQUAL *********************************";
        qDebug() << "mu_0" << mu_0 << "g" << g << "tau_0_a" << tau_0_a << "beta"
                 << beta << "x_m: " << x_m << "x_a: " << x_a << "tau_a" << tau_a
                 << "tau_m" << tau_m;
    };*/
    return result;
}

inline vector<double> compute_B_atm(const std::vector<double> B_sun_list,
                                    const double& mu_0, const double& tau_0_a,
                                    const double& beta, const double& g,
                                    const vector<double>& tau_m,
                                    const vector<double>& list, double Q,
                                    double P, double Tau_e) {
    vector<double> result;
    vector<double> tau_lambda =
        compute_tau_lambda(Tau_e, tau_0_a, beta, tau_m, list);
    vector<double> omega_lambda =
        compute_omega(Tau_e, tau_0_a, beta, tau_m, list);
    vector<double> x = compute_x(mu_0, g, tau_0_a, beta, tau_m, list);

    for (uintmax_t i = 0; i < list.size(); ++i) {  // Check function
        auto b_atm = omega_lambda[i] * B_sun_list[i] * x[i] /
                     (4.0 * (mu + mu_0)) *
                     (1.0 - exp(-tau_lambda[i] * (1.0 / mu_0 + 1.0 / mu))) *
                     (1.0 + Q * pow(omega_lambda[i] * tau_lambda[i], P));
        result.push_back(b_atm);
    }

    omega_lambda_list = omega_lambda;
    tau_lambda_list = tau_lambda;
    x_list = x;
    B_atm_result = result;
    return result;
}

inline double compute_B1(
    const vector<double>& T_O2_list, const double T_O3_list,
    const vector<double>& T_H2O_list, const vector<double>& S_lambda_list,
    const vector<double>& B_lambda_teta_list, const double& mu_0,
    const double& tau_0_a, const double& beta, const double& g,
    const vector<double>& tau_m, const vector<double>& list, double Q, double P,
    double Tau_e) {
    auto B_atm = compute_B_atm(B_lambda_teta_list, mu_0, tau_0_a, beta, g,
                               tau_m, list, Q, P, Tau_e);

    double integral_first = 0.0;
    double integral_second = 0.0;

    for (uintmax_t i = 0; i < list.size(); ++i) {
        integral_first += B_atm[i] * T_H2O_list[i] * S_lambda_list[i];
        integral_second += S_lambda_list[i];
    }
    double B1 = integral_first * T_O3_list / integral_second;
    return B1;
}

inline vector<double> compute_E_lambda(
    const double& mu_0, const double& albedo1, const double albedo2,
    const double& tau_0_a, const double& beta, const double& g,
    const vector<double>& tau_m, const vector<double>& list, double tau_e) {
    vector<double> E;
    vector<double> tau_lambda =
        compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
    vector<double> omega_lambda =
        compute_omega(tau_e, tau_0_a, beta, tau_m, list);
    vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

    for (size_t i = 0; i < list.size(); ++i) {
        double ro_0 =
            compute_ro_0(albedo1, albedo2, list[i], lambda_1, lambda_2);
        auto E_lmb =
            4.0 * pi * omega_lambda[i] * mu_0 * B_lambda_teta_list[i] /
                (4.0 + 3.0 * (1.0 - g_lmb[i]) * (1.0 - ro_0) * tau_lambda[i]) *
                ((0.5 + 0.75 * mu_0) +
                 (0.5 - 0.75 * mu_0) * exp(-tau_lambda[i] / mu_0)) +
            (1.0 - omega_lambda[i]) * pi * B_lambda_teta_list[i] * mu_0 *
                exp(-tau_lambda[i] / mu_0);
        E.push_back(E_lmb);
    }
    e_result = E;
    return E;
}

inline vector<double> compute_E_lambda_final(
    const double& mu_0, const double& ro, const double& tau_0_a,
    const double& beta, const double& g, const vector<double>& tau_m,
    const vector<double>& list, double tau_e) {
    vector<double> E;
    vector<double> tau_lambda =
        compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
    vector<double> omega_lambda =
        compute_omega(tau_e, tau_0_a, beta, tau_m, list);
    vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

    for (size_t i = 0; i < list.size(); ++i) {
        auto E_lmb =
            4.0 * pi * omega_lambda[i] * mu_0 * B_lambda_teta_list[i] /
                (4.0 + 3.0 * (1.0 - g_lmb[i]) * (1.0 - ro) * tau_lambda[i]) *
                ((0.5 + 0.75 * mu_0) +
                 (0.5 - 0.75 * mu_0) * exp(-tau_lambda[i] / mu_0)) +
            (1.0 - omega_lambda[i]) * pi * B_lambda_teta_list[i] * mu_0 *
                exp(-tau_lambda[i] / mu_0);
        E.push_back(E_lmb);
    }
    e_result = E;
    return E;
}

inline vector<double> compute_u(const double& g, const double& tau_0_a,
                                const double& beta, const vector<double>& tau_m,
                                const vector<double>& list) {
    vector<double> u;
    vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

    for (size_t i = 0; i < list.size(); ++i) {
        auto h0 = -1.88227 + 0.53661 * g_lmb[i] - 1.8047 * pow(g_lmb[i], 2) +
                  3.26348 * pow(g_lmb[i], 3) - 2.3 * pow(g_lmb[i], 4);
        auto h1 = 5.97763 - 2.04621 * g_lmb[i] - 2.0173 * pow(g_lmb[i], 2) +
                  1.44843 * pow(g_lmb[i], 3);
        auto h2 = -5.47825 + 2.42154 * g_lmb[i] - 3.37057 * pow(g_lmb[i], 2) +
                  6.13805 * pow(g_lmb[i], 3);
        auto h3 = 2.07593 - 2.03761 * g_lmb[i] + 6.25975 * pow(g_lmb[i], 2) -
                  7.35503 * pow(g_lmb[i], 3);
        u.push_back(h0 + h1 * mu + h2 * (mu * mu) + h3 * (mu * mu * mu));
    }
    return u;
}

inline vector<double> compute_v(const double& g, const double& tau_0_a,
                                const double& beta, const vector<double>& tau_m,
                                const vector<double>& list) {
    vector<double> v;
    vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

    for (size_t i = 0; i < list.size(); ++i) {
        auto ro_0 = 0.4923 + 1.0471 * g_lmb[i] - 2.61112 * pow(g_lmb[i], 2) +
                    1.53155 * pow(g_lmb[i], 3);
        auto ro_1 = 4.01521 - 0.25886 * g_lmb[i] - 2.85378 * pow(g_lmb[i], 2) +
                    3.61515 * pow(g_lmb[i], 3);
        auto ro_2 = 3.76447 + 3.29106 * g_lmb[i] - 12.37951 * pow(g_lmb[i], 2) +
                    9.85 * pow(g_lmb[i], 3);
        v.push_back(ro_0 + ro_1 * exp(-ro_2 * mu));
    }
    return v;
}

inline vector<double> compute_w(const double& g, const double& tau_0_a,
                                const double& beta, const vector<double>& tau_m,
                                const vector<double>& list) {
    vector<double> w;
    vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

    for (size_t i = 0; i < list.size(); ++i) {
        auto q_0 = 0.000076 - 0.316 * g_lmb[i] + 0.67744 * pow(g_lmb[i], 2) -
                   0.4093 * pow(g_lmb[i], 3);
        auto q_1 = -1.31136 - 0.8901 * g_lmb[i] + 3.55 * pow(g_lmb[i], 2) -
                   3.0646 * pow(g_lmb[i], 3);
        auto q_2 = 5.21931 + 7.2255 * g_lmb[i] - 23.43878 * pow(g_lmb[i], 2) +
                   17.65629 * pow(g_lmb[i], 3);
        w.push_back(q_0 + q_1 * exp(-q_2 * mu));
    }
    return w;
}

inline vector<double> compute_T_dif(const double& tau_e, const double& tau_0_a,
                                    const double& beta, const double& g,
                                    const vector<double>& tau_m,
                                    const vector<double>& list) {
    auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
    auto u = compute_u(g, tau_0_a, beta, tau_m, list);
    auto v = compute_v(g, tau_0_a, beta, tau_m, list);
    auto w = compute_w(g, tau_0_a, beta, tau_m, list);
    vector<double> T_dif;

    for (size_t i = 0; i < list.size(); ++i) {
        T_dif.push_back(tau_lambda[i] *
                        exp(-u[i] - v[i] * tau_lambda[i] -
                            w[i] * tau_lambda[i] * tau_lambda[i]));
    }
    return T_dif;
}

inline vector<double> compute_T_lambda(const double& tau_e,
                                       const double& tau_0_a,
                                       const double& beta, const double& g,
                                       const vector<double>& tau_m,
                                       const vector<double>& list) {
    auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
    auto T_dif = compute_T_dif(tau_e, tau_0_a, beta, g, tau_m, list);
    vector<double> T_lmb;

    for (size_t i = 0; i < list.size(); ++i) {
        T_lmb.push_back((exp(-tau_lambda[i]) / mu) + T_dif[i]);
    }
    return T_lmb;
}

inline double compute_B2(const vector<double>& S_lambda_list,
                         const vector<double>& B_lambda_teta_list,
                         const vector<double>& T_O2_list, const double T_O3,
                         const vector<double>& T_H2O_list, const double& mu_0,
                         const double& albedo_1, double albedo_2,
                         const double& tau_0_a, const double& beta,
                         const double& g, const vector<double>& tau_m,
                         const vector<double>& list, double Q, double P,
                         double Tau_e) {
    auto E_lambda = compute_E_lambda(mu_0, albedo_1, albedo_2, tau_0_a, beta, g,
                                     tau_m, list, Tau_e);
    auto T_lambda = compute_T_lambda(Tau_e, tau_0_a, beta, g, tau_m, list);

    double integral_first = 0.0;
    double integral_second = 0.0;

    for (size_t i = 0; i < list.size(); ++i) {
        auto ro_0 =
            compute_ro_0(albedo_1, albedo_2, list[i], lambda_1, lambda_2);
        integral_first += ro_0 / pi * E_lambda[i] * T_lambda[i] *
                          T_H2O_list[i] * S_lambda_list[i];
        integral_second += S_lambda_list[i];
    }

    double B2 = T_O3 * integral_first / integral_second;
    return B2;
}

inline double compute_B2_final(
    const vector<double>& S_lambda_list,
    const vector<double>& B_lambda_teta_list, const vector<double>& T_O2_list,
    const double T_O3, const vector<double>& T_H2O_list, const double& mu_0,
    const double& ro, const double& tau_0_a, const double& beta,
    const double& g, const vector<double>& tau_m, const vector<double>& list,
    double Q, double P, double Tau_e) {
    auto E_lambda =
        compute_E_lambda_final(mu_0, ro, tau_0_a, beta, g, tau_m, list, Tau_e);
    auto T_lambda = compute_T_lambda(Tau_e, tau_0_a, beta, g, tau_m, list);

    double integral_first = 0.0;
    double integral_second = 0.0;

    auto B_atm = compute_B_atm(B_lambda_teta_list, mu_0, tau_0_a, beta, g,
                               tau_m, list, Q, P, Tau_e);

    for (size_t i = 0; i < list.size(); ++i) {
        integral_first += ro / pi * E_lambda[i] * T_lambda[i] * T_H2O_list[i] *
                          S_lambda_list[i];
        integral_second += S_lambda_list[i];
    }

    double B2 = T_O3 * integral_first / integral_second;
    return B2;
}

inline double compute_eq(const double& B1, const double& B2,
                         const double& albedo, const double& divider,
                         const double& dark_pixel) {
    double B = (B1 + B2);  //* albedo / pi);  // /divider
    return dark_pixel - B;
}

inline vector<double> compute_EQ(
    const vector<double>& B_lambda_teta_list, const vector<double>& T_O2_list,
    const vector<double>& T_O3_list, const vector<double>& T_H2O_list,
    const vector<vector<double>>& S_lambda_lists, const double& mu_0,
    const double& albedo_1, const double albedo_2, const double& tau_0_a,
    const double& beta, const double& g, const vector<double>& tau_m,
    const vector<double>& list, const vector<double>& dividers,
    const vector<double>& dark_pixels, double Q, double P, double Tau_e) {
    vector<double> EQ;
    ro_0_list.clear();
    B1_result.clear();
    B2_result.clear();
    B_result.clear();
    for (size_t i = 0; i < dark_pixels.size(); ++i) {
        auto B1 = compute_B1(T_O2_list, T_O3_list[i], T_H2O_list,
                             S_lambda_lists[i], B_lambda_teta_list, mu_0,
                             tau_0_a, beta, g, tau_m, list, Q, P, Tau_e);
        auto B2 = compute_B2(S_lambda_lists[i], B_lambda_teta_list, T_O2_list,
                             T_O3_list[i], T_H2O_list, mu_0, albedo_1, albedo_2,
                             tau_0_a, beta, g, tau_m, list, Q, P, Tau_e);
        double lambda = central_waves[i];
        double ro_0 =
            compute_ro_0(albedo_1, albedo_2, lambda, lambda_1, lambda_2);
        EQ.push_back(compute_eq(B1, B2, ro_0, dividers[i], dark_pixels[i]));
        ro_0_list.push_back(ro_0);
        B1_result.push_back(B1);
        B2_result.push_back(B2);
        B_result.emplace_back(B1 + B2);
    }
    speya_result = EQ;  // last speya_result

    return EQ;
}

inline double compute_mH2O(double B9, double B8a) {
    return (0.588 * B9 - 0.258 * B8a) / (0.00087 * B9 - 0.147 * B8a);
};

inline double compute_TO3(double TiO3, double alfa_i, double X) {
    return TiO3 * std::exp(-alfa_i * (X - 300));
};

inline void compute_TO3_list(double X) {
    for (int i = 0; i < NUMBER_OF_CHANNELS; ++i) {
        T_O3_list[i] = compute_TO3(dobson_TiO[i], dobson_alfa[i], X);
    }
    // qDebug() << "TO3list params:" << X << T_O3_list;
};

inline double compute_ro_0(double ro_1, double ro_2, double lambda,
                           double lambda_1, double lambda_2) {
    return ro_1 + ((ro_2 - ro_1) / (lambda_2 - lambda_1)) * (lambda - lambda_1);
};
// clang-format off
inline double compute_ro(double ro1,  // искомый коэффициент для альбедо
                         double ro2, // искомый коэффициент для альбедо
                         double mu_0,
                         double tau_0_a,
                         double beta,
                         double g,
                         double B1,  // fixed
                         double B,   // pixel band value
                         int band,   // номер канала
                         double Q,
                         double P,
                         double Tau_e) {


    auto B2 = compute_B2(S_lambda_lists[band], B_lambda_teta_list, T_O2_list,
                         T_O3_list[band], T_H2O_list, mu_0, ro1, ro2, tau_0_a,
                         beta, g, tau_m, lambda_list, Q, P, Tau_e);

    double a = (B1 + B2);
    //qDebug()<<band <<"B -- a --> " <<B <<a;
    final_B_result[band] = a;
    return (B - a);
}
// clang-format on

int quadfunc(int m, int n, double* p, double* dy, double** dvec, void* vars) {
    auto X = p[X_INDEX];
    auto Q = p[q_INDEX];
    auto P = p[p_INDEX];
    auto TAU_M_0 = p[tau_mu_0_INDEX];
    auto tau_0_a = p[tau_0_a_INDEX];
    auto beta = p[beta_INDEX];
    auto tau_e = p[tau_e_INDEX];
    auto g = p[g_INDEX];
    auto ro_1 = p[ro_1_INDEX];
    auto ro_2 = p[ro_2_INDEX];

    compute_TO3_list(X);
    tau_m = compute_tau_m(lambda_list, TAU_M_0);

    auto eq = compute_EQ(B_lambda_teta_list, T_O2_list, T_O3_list, T_H2O_list,
                         S_lambda_lists, mu_0, ro_1, ro_2, tau_0_a, beta, g,
                         tau_m, lambda_list, divider_list,
                         origin_speya_pixel_values, Q, P, tau_e);

    for (int i = 0; i < m; i++) {
        dy[i] = eq[i];
    }
    rv.err_X = dy[X_INDEX];
    rv.err_q = dy[q_INDEX];
    rv.err_p = dy[p_INDEX];
    rv.err_tau_mu_0 = dy[tau_mu_0_INDEX];
    rv.err_tau_a0 = dy[tau_0_a_INDEX];
    rv.err_beta = dy[beta_INDEX];
    rv.err_tau_e = dy[tau_e_INDEX];
    rv.err_g = dy[g_INDEX];
    rv.err_albedo_1 = dy[ro_1_INDEX];
    rv.err_albedo_2 = dy[ro_2_INDEX];

    return 0;
}

int albedofunc(int m, int n, double* p, double* dy, double** dvec, void* vars) {
    auto Q = rv.q;
    auto P = rv.p;
    auto TAU_M_0 = rv.tau_mu_0;
    auto tau_0_a = rv.tau_0_a;
    auto beta = rv.beta;
    auto tau_e = rv.tau_e;
    auto g = rv.g;
    auto ro_1 = p[0];
    auto ro_2 = p[1];
    int band = *static_cast<int*>(vars);
    qDebug() << "band: " << band;
    dy[0] = compute_ro(ro_1, ro_2, TAU_M_0, tau_0_a, beta, g, B1_result[band],
                       origin_speya_pixel_values[band], band, Q, P, tau_e);
    return 0;
}

std::vector<double> calculateAlbedoFinal(const QVector<double>& speya_values) {
    if (speya_values.size() != 10) {
        throw std::runtime_error("Количество каналов не равно 10");
        return {};
    }  // TODO exceptions
    origin_speya_pixel_values = speya_values.toStdVector();

    albedo_final_result.clear();

    for (int i = 0; i < NUMBER_OF_CHANNELS; ++i) {
        int status;
        double perror[2]; /* Returned parameter errors */
        mp_par pars[2];   /* Parameter constraints */

        mp_result result;

        memset(&result, 0, sizeof(result)); /* Zero results structure */
        result.xerror = perror;
        memset(pars, 0, sizeof(pars)); /* Initialize constraint structure */
        double p[2];
        p[0] = 0.05;
        // albedo p final
        pars[0].limits[0] = 0.001;
        pars[0].limits[1] = 1;
        pars[0].side = 0;
        pars[0].step = 0.01;
        pars[0].limited[0] = 1;
        pars[0].limited[1] = 1;

        p[1] = 0.05;
        // albedo p final
        pars[1].limits[0] = 0.001;
        pars[1].limits[1] = 1;
        pars[1].side = 0;
        pars[1].step = 0.01;
        pars[1].limited[0] = 1;
        pars[1].limited[1] = 1;
        status = mpfit(albedofunc, 2, 2, p, pars, 0, (void*)&i, &result);
        double ro_1 = p[0];
        double ro_2 = p[1];

        // divider_list[i];//
        double ro_final_in_chanel = 0;
        for (int j = 0; j < lambda_list.size(); ++j) {
            ro_final_in_chanel +=
                S_lambda_lists[i][j] *
                (ro_1 + (ro_2 - ro_1) / (lambda_2 - lambda_1) *
                            (lambda_list[j] - lambda_1));
        }
        albedo_final_result.push_back(ro_final_in_chanel);

        qDebug() << "band: " << i << " ro_value_result: " << ro_final_in_chanel
                 << "\n";
        qDebug() << "band: " << i << "\nALBEDO FINAL STATUS: " << status;
    }

    return albedo_final_result;
}

}  // namespace

namespace lss {

void setSunZenitAngle(const double& angle) {
    mu_0 = qCos(qDegreesToRadians(angle));
    qDebug() << "sun zenit angle: " << angle << "  cos mu_0: " << mu_0;
}

void setCaptureZenitAngle(const double& angle) {
    mu = qCos(qDegreesToRadians(angle));
    qDebug() << "capture zenit angle: " << angle << "  cos mu: " << mu;
}

void setFiAngle(double angleSA, double angleCA) {
    fi = qCos(qDegreesToRadians(angleCA - angleSA));
    qDebug() << "fi angle: " << angleCA - angleSA << "  cos fi: " << fi;
}

inline double compute_gamma(double mu, double mu_0, double fi) {
    gamma = -mu * mu_0 + sqrt((1 - mu * mu) * (1 - mu_0 * mu_0)) * fi;
    qDebug() << "gamma value: " << gamma;
    return gamma;
}

void updateSatelliteResponses(const QString& satellite_name) {
    //"sentinel 2A", "sentinel 2B"
    qDebug() << "Update satellite name...." << satellite_name;
    if (satellite_name_key == satellite_name) {
        qDebug() << "We do not need to update satellite responses...";
        return;
    }
    satellite_name_key = satellite_name;
    QJsonArray sat_responses;
    if (satellite_name == "sentinel 2A") {
        sat_responses = sat_sentinel2A_respns;
    } else if (satellite_name == "sentinel 2B") {
        sat_responses = sat_sentinel2B_respns;
    } else {
        Q_ASSERT(false);
        qDebug() << "UKNOWN SATTELITE NAME";
    }
    S_lambda_lists.clear();
    std::vector<double> full_values(601, 0);
    qDebug() << "sentinel_responses size..." << sat_responses.size();
    for (int j = 0; j < sat_responses.size(); ++j) {
        auto arr = sat_responses[j].toObject()["spectral_response"].toArray();
        int wave_offset =
            sat_responses[j].toObject()["wavelength_MIN_nm"].toInt();
        if (wave_offset > 1000) continue;
        for (int i = 0; i < arr.size(); ++i) {
            int index = wave_offset - 400 + i;
            if (index > 600) break;
            full_values[index] = arr[i].toDouble();
        };
        S_lambda_lists.push_back(full_values);
        full_values.assign(601, 0.0);
    }
    qDebug() << "Responses: " << S_lambda_lists[0].size()
             << S_lambda_lists.size();
    calculDividerList(S_lambda_lists);
}

result_values optimize(const QString& sat_name,
                       const QVector<double>& speya_values,
                       const QVector<double>& initial_values) {
    if (is_first_run) {
        loadAllLists();
        is_first_run = false;
    }
    if (sat_name != satellite_name_key) updateSatelliteResponses(sat_name);
    if (initial_values.size() != 10) return rv;
    /* Initial conditions */

    double p[10];
    for (int i = 0; i < initial_values.size(); ++i) {
        p[i] = initial_values[i];
    }

    if (speya_values.size() != 10) {
        throw std::runtime_error("Количество каналов не равно 10");
        return rv;
    }  // TODO exceptions
    origin_speya_pixel_values = speya_values.toStdVector();
    double perror[10]; /* Returned parameter errors */
    mp_par pars[10];   /* Parameter constraints */
    vars_struct v;
    int status;
    mp_result result;

    memset(&result, 0, sizeof(result)); /* Zero results structure */
    result.xerror = perror;
    memset(pars, 0, sizeof(pars)); /* Initialize constraint structure */

    // X
    pars[X_INDEX].limits[0] = 250;
    pars[X_INDEX].limits[1] = 350;
    pars[X_INDEX].limited[0] = 1;
    pars[X_INDEX].limited[1] = 1;
    char X_param_name[] = "X";
    pars[X_INDEX].parname = X_param_name;
    pars[X_INDEX].side = 0;
    pars[X_INDEX].step = 1;

    // q
    pars[q_INDEX].limits[0] = 1;
    pars[q_INDEX].limits[1] = 6;
    pars[q_INDEX].limited[0] = 1;
    pars[q_INDEX].limited[1] = 1;
    pars[q_INDEX].side = 0;
    pars[q_INDEX].step = 0.01;

    // p
    pars[p_INDEX].limits[0] = 1;
    pars[p_INDEX].limits[1] = 1.8;
    pars[p_INDEX].limited[0] = 1;
    pars[p_INDEX].limited[1] = 1;
    pars[p_INDEX].side = 0;
    pars[p_INDEX].step = 0.01;

    // tau_m_0
    pars[tau_mu_0_INDEX].limits[0] = 0.09;
    pars[tau_mu_0_INDEX].limits[1] = 0.15;
    pars[tau_mu_0_INDEX].limited[0] = 1;
    pars[tau_mu_0_INDEX].limited[1] = 1;
    pars[tau_mu_0_INDEX].side = 0;
    pars[tau_mu_0_INDEX].step = 0.01;

    // tau_a_0
    pars[tau_0_a_INDEX].limits[0] = 0.1;
    pars[tau_0_a_INDEX].limits[1] = 1.3;
    pars[tau_0_a_INDEX].limited[0] = 1;
    pars[tau_0_a_INDEX].limited[1] = 1;
    pars[tau_0_a_INDEX].side = 0;
    pars[tau_0_a_INDEX].step = 0.01;

    // beta
    pars[beta_INDEX].limits[0] = 0.001;
    pars[beta_INDEX].limits[1] = 2.8;
    pars[beta_INDEX].side = 0;
    pars[beta_INDEX].step = 0.01;
    pars[beta_INDEX].limited[0] = 1;
    pars[beta_INDEX].limited[1] = 1;

    // tau_e
    pars[tau_e_INDEX].limits[0] = 0.001;
    pars[tau_e_INDEX].limits[1] = 0.1;
    pars[tau_e_INDEX].side = 0;
    pars[tau_e_INDEX].step = 0.01;
    pars[tau_e_INDEX].limited[0] = 1;
    pars[tau_e_INDEX].limited[1] = 1;

    // g
    pars[g_INDEX].limits[0] = 0.1;
    pars[g_INDEX].limits[1] = 0.8;
    pars[g_INDEX].side = 0;
    pars[g_INDEX].step = 0.001;
    pars[g_INDEX].limited[0] = 1;
    pars[g_INDEX].limited[1] = 1;

    // albedo p1
    pars[ro_1_INDEX].limits[0] = 0.001;
    pars[ro_1_INDEX].limits[1] = 0.8;
    pars[ro_1_INDEX].side = 0;
    pars[ro_1_INDEX].step = 0.01;
    pars[ro_1_INDEX].limited[0] = 1;
    pars[ro_1_INDEX].limited[1] = 1;

    // albedo p2
    pars[ro_2_INDEX].limits[0] = 0.001;
    pars[ro_2_INDEX].limits[1] = 0.8;
    pars[ro_2_INDEX].side = 0;
    pars[ro_2_INDEX].step = 0.01;
    pars[ro_2_INDEX].limited[0] = 1;
    pars[ro_2_INDEX].limited[1] = 1;

    status = mpfit(quadfunc, 10, 10, p, pars, 0, (void*)&v, &result);
    QStringList params = {"X",    "q",     "p", "tau_mu_0", "tau_0_a",
                          "beta", "tau_e", "g", "ro_1",     "ro_2"};
    qDebug() << "\nSTATUS: " << status;
    qDebug() << "VALUES: ";
    for (int i = 0; i < NUMBER_OF_CHANNELS; ++i) {
        qDebug() << params[i] << ": " << p[i];
    }
    qDebug() << "--------------------------------------------------------------"
                "--------";
    // clang-format off
    /*qDebug() << "ERROR: "  <<"X            : "<< result.xerror[0]<<"\n"
                           <<"q            : "<< result.xerror[1]<<"\n"
                           <<"p            : "<< result.xerror[2]<<"\n"
                           <<"tau_mu_0     : "<< result.xerror[3]<<"\n"
                           <<"err_tau_a0   : "<< result.xerror[4]<<"\n"
                           <<"beta         : "<< result.xerror[5]<<"\n"
                           <<"err_tau_e    : "<< result.xerror[6]<<"\n"
                           <<"err_g        : "<< result.xerror[7]<<"\n"
                           <<"err_albedo_1 : "<< result.xerror[8]<<"\n"
                           <<"err_albedo_2 : "<< result.xerror[9]<<"\n";*/
    // clang-format on

    rv.X = p[X_INDEX];
    rv.q = p[q_INDEX];
    rv.p = p[p_INDEX];
    rv.tau_mu_0 = p[tau_mu_0_INDEX];
    rv.tau_0_a = p[tau_0_a_INDEX];
    rv.beta = p[beta_INDEX];
    rv.tau_e = p[tau_e_INDEX];
    rv.g = p[g_INDEX];
    rv.albedo_1 = p[ro_1_INDEX];
    rv.albedo_2 = p[ro_2_INDEX];

    // show last speya result
    dv::Config cfg;
    dv::holdOn();
    dv::show(v_central_waves, origin_speya_pixel_values, "Origin");
    dv::show(v_central_waves, B_result, "Fitted");
    dv::holdOff();
    // dv::show(lambda_list, e_result, "E");
    // dv::show(lambda_list, B_atm_result, "B_atm");
    // dv::show(lambda_list, B_lambda_teta_list, "B_lambda_teta_list");
    // dv::show(lambda_list, omega_lambda_list, "omega_lambda");
    dv::show(lambda_list, tau_lambda_list, "tau_lambda");
    dv::show(lambda_list, x_list, "x_list");

    // dv::show(v_central_waves, ro_0_list, "ro_0_list");
    //  dv::show(v_central_waves, B1_result, "B1_list");
    //  dv::show(v_central_waves, B2_result, "B2_list");
    qDebug() << "mu: " << mu;
    qDebug() << "mu_0: " << mu_0;
    qDebug() << "gamma: " << gamma;
    qDebug() << "x_m_res: " << x_m_res;
    qDebug() << "x_a_res: " << x_a_res;

    std::vector<double> x_check_values;

    for (size_t i = 0; i < lambda_list.size(); i++) {
        double sum = tau_m_res[i] + tau_a_res[i];
        auto value =
            x_m_res * tau_m_res[i] / (sum) + x_a_res * tau_a_res[i] / (sum);
        x_check_values.push_back(value);
    }

    dv::show(lambda_list, tau_a_res, "tau_a");
    dv::show(lambda_list, tau_m_res, "tau_m");
    dv::show(lambda_list, x_check_values, "x_check_list");

    auto sv = QVector<double>::fromStdVector(origin_speya_pixel_values);
    auto albedo_pixel = calculateAlbedoFinal(sv);
    qDebug() << "albedo: " << albedo_pixel;
    dv::show(v_central_waves, albedo_pixel, "final_albedo");
    dv::holdOn();
    dv::show(v_central_waves, origin_speya_pixel_values, "Origin");
    dv::show(v_central_waves, final_B_result, "Fitted");
    dv::holdOff();

    return rv;
}

}  // namespace lss
