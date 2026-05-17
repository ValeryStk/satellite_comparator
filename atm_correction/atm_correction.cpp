#include "atm_correction.h"

#include <stdio.h>
#include <stdlib.h>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QtMath>
#include <array>
#include <cmath>
#include <vector>

#include "common_types.h"
#include "json_utils.h"
#include "math.h"
#include "mpfit.h"

namespace lss {
void updateSatelliteResponses(const QString& satellite_name);
}

namespace {
using std::string;
using std::vector;

static bool is_first_run = true;
constexpr uint16_t NUMBER_WAVELENGTH = 601;
constexpr double TAU_M_0 = 0.101;
constexpr double LAMBDA_0 = 0.55;
constexpr double P = 1.25;
constexpr double Q = 1;
constexpr double TAU_E = 0.04;
constexpr double pi = 3.14159265358979323846;

struct vars_struct {
    double* tau_0_a_err;
    double* beta_err;
    double* g_err;
    double* albedo_err;
};

double mu_0 = qCos(qDegreesToRadians(41.3));
static result_values rv;
void calculDividerList(vector<vector<double>>& responses);
inline vector<double> compute_tau_m(const vector<double>& list);

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
        double h2o = atm_params[i].toObject()["h2o"].toDouble();
        T_H2O_list.push_back(h2o);
        double o2 = atm_params[i].toObject()["o2"].toDouble();
        T_O2_list.push_back(o2);
        double o3 = atm_params[i].toObject()["o3"].toDouble();
        T_O3_list.push_back(o3);
        double wl = atm_params[i].toObject()["wavelength"].toDouble();
        lambda_list.push_back(wl);
        double sun = atm_params[i].toObject()["sun"].toDouble();
        B_lambda_teta_list.push_back(sun);
    }
    tau_m = compute_tau_m(lambda_list);
    satellite_name_key = "";
    lss::updateSatelliteResponses("sentinel 2A");
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
    for (int i = 0; i < divider_list.size(); ++i) {
        qDebug() << "DIVIDED: " << divider_list[i];
    }
}

inline vector<double> compute_tau_m(const vector<double>& list) {
    std::vector<double> result;
    for (uintmax_t i = 0; i < list.size(); ++i) {
        auto lambda_0_lambda = LAMBDA_0 / list[i];
        result.push_back(TAU_M_0 * pow(lambda_0_lambda, 4));
    }
    return result;
}

inline double compute_x_m(const double& mu_0) {
    return 3 * (1 + pow(mu_0, 2)) / 4;
}

inline double compute_x_a(const double& mu_0, const double& g) {
    return (1 - pow(g, 2)) / pow((1 + pow(g, 2) + 2 * g * mu_0), 1.5);
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
    auto x_m = compute_x_m(mu_0);
    auto x_a = compute_x_a(mu_0, g);
    for (uintmax_t i = 0; i < list.size(); ++i) {
        result.push_back((x_m * tau_m[i] + x_a * tau_a[i]) /
                         (tau_m[i] + tau_a[i]));
    }
    return result;
}

inline vector<double> compute_B_atm(const double& mu_0, const double& tau_0_a,
                                    const double& beta, const double& g,
                                    const vector<double>& tau_m,
                                    const vector<double>& list) {
    vector<double> result;
    vector<double> tau_lambda =
        compute_tau_lambda(TAU_E, tau_0_a, beta, tau_m, list);
    vector<double> omega_lambda =
        compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
    vector<double> x = compute_x(mu_0, g, tau_0_a, beta, tau_m, list);

    for (uintmax_t i = 0; i < list.size(); ++i) {
        auto b_atm = omega_lambda[i] * x[i] / (4.0 * (1.0 + mu_0)) *
                     (1.0 - exp(-tau_lambda[i] * (1.0 / mu_0 + 1.0))) *
                     (1.0 + Q * pow(omega_lambda[i] * tau_lambda[i], P));
        result.push_back(b_atm);
    }
    return result;
}

inline double compute_B1(
    const vector<double>& T_O2_list, const vector<double>& T_O3_list,
    const vector<double>& T_H2O_list, const vector<double>& S_lambda_list,
    const vector<double>& B_lambda_teta_list, const double& mu_0,
    const double& tau_0_a, const double& beta, const double& g,
    const vector<double>& tau_m, const vector<double>& list) {
    double B1 = 0.0;
    auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
    for (uintmax_t i = 0; i < list.size(); ++i) {
        auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
        auto S_lambda = S_lambda_list[i];
        auto B_sun = B_lambda_teta_list[i];
        auto b = B_atm[i];
        B1 += b * T_g_lambda * S_lambda * B_sun;
    }
    return B1;
}

inline vector<double> compute_E_lambda(const double& mu_0, const double& albedo,
                                       const double& tau_0_a,
                                       const double& beta, const double& g,
                                       const vector<double>& tau_m,
                                       const vector<double>& list) {
    vector<double> E;
    vector<double> tau_lambda =
        compute_tau_lambda(TAU_E, tau_0_a, beta, tau_m, list);
    vector<double> omega_lambda =
        compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
    vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

    for (size_t i = 0; i < list.size(); ++i) {
        auto E_lmb =
            4.0 * pi * omega_lambda[i] * mu_0 /
                (4.0 +
                 3.0 * (1.0 - g_lmb[i]) * (1.0 - albedo) * tau_lambda[i]) *
                ((0.5 + 0.75 * mu_0) +
                 (0.5 - 0.75 * mu_0) * exp(-tau_lambda[i] / mu_0)) +
            (1.0 - omega_lambda[i]) * pi * mu_0 * exp(-tau_lambda[i] / mu_0);
        E.push_back(E_lmb);
    }
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
        u.push_back(h0 + h1 + h2 + h3);
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
        v.push_back(ro_0 + ro_1 * exp(-ro_2));
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
        w.push_back(q_0 + q_1 * exp(-q_2));
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
        T_lmb.push_back((exp(-tau_lambda[i])) + T_dif[i]);
    }
    return T_lmb;
}

inline double compute_B2(const vector<double>& S_lambda_list,
                         const vector<double>& B_lambda_teta_list,
                         const vector<double>& T_O2_list,
                         const vector<double>& T_O3_list,
                         const vector<double>& T_H2O_list, const double& mu_0,
                         const double& albedo, const double& tau_0_a,
                         const double& beta, const double& g,
                         const vector<double>& tau_m,
                         const vector<double>& list) {
    auto E_lambda =
        compute_E_lambda(mu_0, albedo, tau_0_a, beta, g, tau_m, list);
    auto T_lambda = compute_T_lambda(TAU_E, tau_0_a, beta, g, tau_m, list);
    double B2 = 0.0;
    auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
    for (size_t i = 0; i < list.size(); ++i) {
        auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
        auto S_lambda = S_lambda_list[i];
        auto B_sun = B_lambda_teta_list[i];
        auto T = T_lambda[i];
        auto E = E_lambda[i];
        B2 += E * T * T_g_lambda * S_lambda * B_sun;
    }
    return B2;
}

inline double compute_eq(const double& B1, const double& B2,
                         const double& albedo, const double& divider,
                         const double& dark_pixel) {
    double B = (B1 + B2 * albedo / pi) / divider;
    return dark_pixel - B;
}

inline vector<double> compute_EQ(
    const vector<double>& B_lambda_teta_list, const vector<double>& T_O2_list,
    const vector<double>& T_O3_list, const vector<double>& T_H2O_list,
    const vector<vector<double>>& S_lambda_lists, const double& mu_0,
    const double& albedo, const double& tau_0_a, const double& beta,
    const double& g, const vector<double>& tau_m, const vector<double>& list,
    const vector<double>& dividers, const vector<double>& dark_pixels) {
    vector<double> EQ;
    for (size_t i = 0; i < dark_pixels.size(); ++i) {
        auto B1 =
            compute_B1(T_O2_list, T_O3_list, T_H2O_list, S_lambda_lists[i],
                       B_lambda_teta_list, mu_0, tau_0_a, beta, g, tau_m, list);
        auto B2 = compute_B2(S_lambda_lists[i], B_lambda_teta_list, T_O2_list,
                             T_O3_list, T_H2O_list, mu_0, albedo, tau_0_a, beta,
                             g, tau_m, list);
        EQ.push_back(compute_eq(B1, B2, albedo, dividers[i], dark_pixels[i]));
    }
    return EQ;
}

inline double compute_ro(double ro,  // albedo
                         double mu_0, double tau_0_a, double beta, double g,
                         double B1,   // fixed
                         double B,    // pixel band value
                         int band) {  // chanel number

    auto B2 = compute_B2(S_lambda_lists[band], B_lambda_teta_list, T_O2_list,
                         T_O3_list, T_H2O_list, mu_0, ro, tau_0_a, beta, g,
                         tau_m, lambda_list) *
              ro / pi;
    double a = (B1 + B2) / divider_list[band];
    return (B - a);
}

int quadfunc(int m, int n, double* p, double* dy, double** dvec, void* vars) {
    auto tau_0_a = p[0];
    auto beta = p[1];
    auto g = p[2];
    auto albedo = p[3];

    auto eq = compute_EQ(B_lambda_teta_list, T_O2_list, T_O3_list, T_H2O_list,
                         S_lambda_lists, mu_0, albedo, tau_0_a, beta, g, tau_m,
                         lambda_list, divider_list, dark_pixels);

    for (int i = 0; i < m; i++) {
        dy[i] = eq[i];
    }
    rv.err_tau = dy[0];
    rv.err_beta = dy[1];
    rv.err_g = dy[2];
    rv.err_albedo = dy[3];
    return 0;
}

int albedofunc(int m, int n, double* p, double* dy, double** dvec, void* vars) {
    auto tau_0_a = p[0];
    auto beta = p[1];
    auto g = p[2];
    auto ro = p[3];

    auto eq = compute_ro(ro, mu_0, tau_0_a, beta, g, ro_fin.B1,
                         ro_fin.band_value, ro_fin.band_number);
    dy[3] = eq;
    rv.err_tau = dy[0];
    rv.err_beta = dy[1];
    rv.err_g = dy[2];
    rv.err_albedo = dy[3];
    return 0;
}
}  // namespace

namespace lss {

void setSunZenitAngle(const double& angle) {
    mu_0 = qCos(qDegreesToRadians(angle));
    qDebug() << "sun zenit angle: " << angle;
    qDebug() << "cos mu_0: " << mu_0;
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
    }
    qDebug() << "Responses: " << S_lambda_lists[0].size();
    calculDividerList(S_lambda_lists);
}

result_values optimize(const QString& sat_name, const QVector<double>& blacks) {
    if (is_first_run) {
        loadAllLists();
        is_first_run = false;
    }
    if (sat_name != satellite_name_key) updateSatelliteResponses(sat_name);
    double p[] = {
        0.1, 2, 0.01,
        0.01};  //{0.1, 2, 0.01, 0.01};               /* Initial conditions */
    if (blacks.size() != 4) return rv;  // TODO exceptions
    dark_pixels = {blacks[0], blacks[1], blacks[2],
                   blacks[3]};  //{36.525799,24.058825,11.294599,4.025315};
    double perror[4];           /* Returned parameter errors */
    mp_par pars[4];             /* Parameter constraints */
    vars_struct v;
    int status;
    mp_result result;

    memset(&result, 0, sizeof(result)); /* Zero results structure */
    result.xerror = perror;
    memset(pars, 0, sizeof(pars)); /* Initialize constraint structure */

    //[0.1, 2, 0.1, 0.01]), bounds=([0, 0.1, 0.1, 0.001], [1, 4, 1, 0.5]))

    // tau_0_a
    pars[0].limits[0] = 0;
    pars[0].limits[1] = 1;
    pars[0].limited[0] = 1;
    pars[0].limited[1] = 1;
    pars[0].side = 0;
    pars[0].step = 0.01;

    // beta
    pars[1].limits[0] = 0.001;
    pars[1].limits[1] = 4;
    pars[1].side = 0;
    pars[1].step = 0.01;
    pars[1].limited[0] = 1;
    pars[1].limited[1] = 1;

    // g
    pars[2].limits[0] = 0.0001;
    pars[2].limits[1] = 1;
    pars[2].side = 0;
    pars[2].step = 0.001;
    pars[2].limited[0] = 1;
    pars[2].limited[1] = 1;

    // albedo
    pars[3].limits[0] = 0.001;
    pars[3].limits[1] = 0.5;
    pars[3].side = 0;
    pars[3].step = 0.01;
    pars[3].limited[0] = 1;
    pars[3].limited[1] = 1;

    status = mpfit(quadfunc, 4, 4, p, pars, 0, (void*)&v, &result);

    qDebug() << "\nSTATUS: " << status;
    qDebug() << "VALUES: " << p[0] << p[1] << p[2] << p[3];
    qDebug() << "ERROR: " << rv.err_tau << rv.err_beta << rv.err_g
             << rv.err_albedo;

    rv.tau_0_a = p[0];
    rv.beta = p[1];
    rv.g = p[2];
    rv.albedo = p[3];

    return rv;
}

double calculateAlbedo(const double tau, const double beta, const double g,
                       const double band_number, const double band_value) {
    double B1 = compute_B1(T_O2_list, T_O3_list, T_H2O_list,
                           S_lambda_lists[band_number], B_lambda_teta_list,
                           mu_0, tau, beta, g, tau_m, lambda_list);
    ro_fin.B1 = B1;
    ro_fin.band_number = band_number;
    ro_fin.band_value = band_value;
    double p[] = {tau, beta, g, 0.01};
    double perror[4]; /* Returned parameter errors */
    mp_par pars[4];   /* Parameter constraints */
    vars_struct v;
    int status;
    mp_result result;

    memset(&result, 0, sizeof(result)); /* Zero results structure */
    result.xerror = perror;
    memset(pars, 0, sizeof(pars)); /* Initialize constraint structure */

    // tau_0_a
    pars[0].fixed = 1;
    // beta
    pars[1].fixed = 1;
    // g
    pars[2].fixed = 1;
    // albedo
    pars[3].limits[0] = 0.001;
    pars[3].limits[1] = 1;
    pars[3].side = 0;
    pars[3].step = 0.01;
    pars[3].limited[0] = 1;
    pars[3].limited[1] = 1;

    status = mpfit(albedofunc, 4, 4, p, pars, 0, (void*)&v, &result);

    // qDebug() << "\nSTATUS: " << status;
    // qDebug() << "VALUES: " << p[0] << p[1] << p[2] << p[3];
    // qDebug() << "ERROR: " << rv.err_tau << rv.err_beta << rv.err_g <<
    // rv.err_albedo;

    double albedo = p[3];
    return albedo;
}

}  // namespace lss
