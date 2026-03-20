#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <vector>
#include <math.h>
#include <cmath>

constexpr double TAU_M_0 = 0.101;
constexpr double LAMBDA_0 = 0.55;
constexpr double P = 1.25;
constexpr double Q = 1;
constexpr double TAU_E = 0.04;
constexpr double pi = 3.14159265358979323846;


using std::vector;


inline vector <double> compute_tau_m(const vector<double>& list) {
  std::vector <double> result;
  for (int i = 0; i < list.size(); ++i) {
    auto lambda_0_lambda = LAMBDA_0 / list[i];
    result.push_back(TAU_M_0 * pow(lambda_0_lambda, 4));
  }
  return result;
}

inline double compute_x_m(const double& mu_0) {
  return  3 * (1 + pow(mu_0, 2)) / 4;
}

inline double compute_x_a(const double& mu_0, const double& g) {
  return (1 - pow(g, 2)) / pow((1 + pow(g, 2) + 2 * g * mu_0), 1.5);
}

inline vector <double> compute_tau_a(const double& tau_0_a,
                                     const double& beta,
                                     const vector <double>& list) {
  std::vector <double> result;
  for (int i = 0; i < list.size(); ++i) {
    double lambda_0_lambda = LAMBDA_0 / list[i];
    result.push_back(tau_0_a * pow(lambda_0_lambda, beta));
  }
  return result;
}


inline vector <double> compute_tau_lambda(const double& tau_e,
                                          const double& tau_0_a,
                                          const double& beta,
                                          const vector <double>& tau_m,
                                          const vector <double>& list) {
  std::vector <double> result;
  std::vector <double> tau_a = compute_tau_a(tau_0_a, beta, list);
  for (int i = 0; i < list.size(); ++i) {
    result.push_back(tau_m[i] + tau_a[i] + tau_e);
  }
  return result;
}

inline vector <double> compute_omega(const double& tau_e,
                                     const double& tau_0_a,
                                     const double& beta,
                                     const vector <double>& tau_m,
                                     const vector <double>& list) {
  std::vector<double> result;
  std::vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
  for (int i = 0; i < list.size(); ++i) {
    result.push_back((tau_m[i] + tau_a[i]) / (tau_m[i] + tau_a[i] + tau_e));
  }
  return result;
}

inline vector <double> compute_g(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> result;
  vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
  for (int i = 0; i < list.size(); ++i) {
    result.push_back(g * tau_a[i] / (tau_m[i] + tau_a[i]));
  }
  return result;
}

inline vector <double> compute_x(const double& mu_0,
                                 const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> result;
  vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
  auto x_m = compute_x_m(mu_0);
  auto x_a = compute_x_a(mu_0, g);
  for (int i = 0; i < list.size(); ++i) {
    result.push_back((x_m * tau_m[i] + x_a * tau_a[i]) / (tau_m[i] + tau_a[i]));
  }
  return result;
}

inline vector <double> compute_B_atm(const double& mu_0,
                                     const double& tau_0_a,
                                     const double& beta,
                                     const double& g,
                                     const vector <double>& tau_m,
                                     const vector <double>& list) {
  vector<double> result;
  vector<double> tau_lambda = compute_tau_lambda(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> omega_lambda = compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> x = compute_x(mu_0, g, tau_0_a, beta, tau_m, list);

  for (int i = 0; i < list.size(); ++i) {
    auto b_atm = omega_lambda[i] * x[i] / (4.0 * (1.0 + mu_0)) * (1.0 - exp(-tau_lambda[i] * (
                                                                                1.0 / mu_0 + 1.0))) * (1.0 + Q * pow(omega_lambda[i] * tau_lambda[i], P));
    result.push_back(b_atm);

  }
  return result;
}

inline double compute_B1(const vector <double>& T_O2_list,
                         const vector <double>& T_O3_list,
                         const vector <double>& T_H2O_list,
                         const vector <double>& S_lambda_list,
                         const vector <double>& B_lambda_teta_list,
                         const double& mu_0,
                         const double& tau_0_a,
                         const double& beta,
                         const double& g,
                         const vector <double>& tau_m,
                         const vector <double>& list) {
  double B1 = 0.0;
  auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
  for (int i = 0; i < list.size(); ++i) {
    auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
    auto S_lambda = S_lambda_list[i];
    auto B_sun = B_lambda_teta_list[i];
    auto b = B_atm[i];
    B1 += b * T_g_lambda * S_lambda * B_sun;
  }
  return B1;
}

inline vector <double> compute_E_lambda(const double& mu_0,
                                        const double& albedo,
                                        const double& tau_0_a,
                                        const double& beta,
                                        const double& g,
                                        const vector <double>& tau_m,
                                        const vector <double>& list) {
  vector<double> E;
  vector<double> tau_lambda = compute_tau_lambda(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> omega_lambda = compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (int i = 0; i < list.size(); ++i) {
    auto E_lmb = 4.0 * pi * omega_lambda[i]
                 * mu_0 /
                 (4.0 + 3.0 * (1.0 - g_lmb[i]) * (1.0 - albedo) * tau_lambda[i])
                 * ((0.5 + 0.75 * mu_0) + (0.5 - 0.75 * mu_0) * exp(-tau_lambda[i] / mu_0))
                 + (1.0 - omega_lambda[i]) * pi * mu_0 * exp(-tau_lambda[i] / mu_0);
    E.push_back(E_lmb);

  }
  return E;
}



inline vector <double> compute_u(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> u;
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (int i = 0; i < list.size(); ++i) {
    auto h0 = -1.88227 + 0.53661 * g_lmb[i] - 1.8047 * pow(g_lmb[i], 2) +
              3.26348 * pow(g_lmb[i], 3) - 2.3 * pow(g_lmb[i], 4);
    auto h1 = 5.97763 - 2.04621 * g_lmb[i] - 2.0173 *
              pow(g_lmb[i], 2) + 1.44843 * pow(g_lmb[i], 3);
    auto h2 = -5.47825 + 2.42154 * g_lmb[i] - 3.37057 *
              pow(g_lmb[i], 2) + 6.13805 * pow(g_lmb[i], 3);
    auto h3 = 2.07593 - 2.03761 * g_lmb[i] + 6.25975 *
              pow(g_lmb[i], 2) - 7.35503 * pow(g_lmb[i], 3);
    u.push_back(h0 + h1 + h2 + h3);
  }
  return u;
}


inline vector <double> compute_v(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> v;
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (int i = 0; i < list.size(); ++i) {
    auto ro_0 = 0.4923 + 1.0471 * g_lmb[i] - 2.61112 *
                pow(g_lmb[i], 2) + 1.53155 * pow(g_lmb[i], 3);
    auto ro_1 = 4.01521 - 0.25886 * g_lmb[i] - 2.85378 *
                pow(g_lmb[i], 2) + 3.61515 * pow(g_lmb[i], 3);
    auto ro_2 = 3.76447 + 3.29106 * g_lmb[i] - 12.37951 *
                pow(g_lmb[i], 2) + 9.85 * pow(g_lmb[i], 3);
    v.push_back(ro_0 + ro_1 * exp(-ro_2));
  }
  return v;
}


inline vector <double> compute_w(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> w;
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (int i = 0; i < list.size(); ++i) {
    auto q_0 = 0.000076 - 0.316 * g_lmb[i] + 0.67744 * \
               pow(g_lmb[i], 2) - 0.4093 * pow(g_lmb[i], 3);
    auto q_1 = -1.31136 - 0.8901 * g_lmb[i] + 3.55 * \
               pow(g_lmb[i], 2) - 3.0646 * pow(g_lmb[i], 3);
    auto q_2 = 5.21931 + 7.2255 * g_lmb[i] - 23.43878 * \
               pow(g_lmb[i], 2) + 17.65629 * pow(g_lmb[i], 3);
    w.push_back(q_0 + q_1 * exp(-q_2));
  }
  return w;
}



inline vector <double> compute_T_dif(const double& tau_e,
                                     const double& tau_0_a,
                                     const double& beta,
                                     const double& g,
                                     const vector <double>& tau_m,
                                     const vector <double>& list) {

  auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
  auto u = compute_u(g, tau_0_a, beta, tau_m, list);
  auto v = compute_v(g, tau_0_a, beta, tau_m, list);
  auto w = compute_w(g, tau_0_a, beta, tau_m, list);
  vector<double> T_dif;

  for (int i = 0; i < list.size(); ++i) {
    T_dif.push_back(tau_lambda[i] * exp(-u[i] - v[i] * tau_lambda[i] - w[i] * tau_lambda[i] * tau_lambda[i]));
  }
  return T_dif;
}


inline vector <double> compute_T_lambda(const double& tau_e,
                                        const double& tau_0_a,
                                        const double& beta,
                                        const double& g,
                                        const vector <double>& tau_m,
                                        const vector <double>& list) {

  auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
  auto T_dif = compute_T_dif(tau_e, tau_0_a, beta, g, tau_m, list);
  vector<double> T_lmb;

  for (int i = 0; i < list.size(); ++i) {
    T_lmb.push_back((exp(-tau_lambda[i])) + T_dif[i]);
  }
  return T_lmb;
}


inline double compute_B2(const vector <double>& S_lambda_list,
                         const vector <double>& B_lambda_teta_list,
                         const vector <double>& T_O2_list,
                         const vector <double>& T_O3_list,
                         const vector <double>& T_H2O_list,
                         const double& mu_0,
                         const double& albedo,
                         const double& tau_0_a,
                         const double& beta,
                         const double& g,
                         const vector <double>& tau_m,
                         const vector <double>& list) {
  auto E_lambda = compute_E_lambda(mu_0, albedo, tau_0_a, beta, g, tau_m, list);
  auto T_lambda = compute_T_lambda(TAU_E, tau_0_a, beta, g, tau_m, list);
  double B2 = 0.0;
  auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
  for (int i = 0; i < list.size(); ++i) {
    auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
    auto S_lambda = S_lambda_list[i];
    auto B_sun = B_lambda_teta_list[i];
    auto T = T_lambda[i];
    auto E = E_lambda[i];
    B2 += E * T * T_g_lambda * S_lambda * B_sun;
  }
  return B2;
}



inline double compute_eq(const double& B1,
                         const double& B2,
                         const double& albedo,
                         const double& divider,
                         const double& dark_pixel) {
  double B = (B1 + B2 * albedo / pi) / divider;
  return dark_pixel - B;
}


inline vector <double> compute_EQ(const vector <double>& B_lambda_teta_list,
                                  const vector <double>& T_O2_list,
                                  const vector <double>& T_O3_list,
                                  const vector <double>& T_H2O_list,
                                  const vector <vector<double>>& S_lambda_lists,
                                  const double& mu_0,
                                  const double& albedo,
                                  const double& tau_0_a,
                                  const double& beta,
                                  const double& g,
                                  const vector <double>& tau_m,
                                  const vector <double>& list,
                                  const vector <double>& dividers,
                                  const vector <double>& dark_pixels) {

  vector <double> EQ;
  for (int i = 0; i < dark_pixels.size(); ++i) {
    auto B1 = compute_B1(T_O2_list, T_O3_list, T_H2O_list, S_lambda_lists[i],
                         B_lambda_teta_list, mu_0, tau_0_a, beta, g, tau_m, list);
    auto B2 = compute_B2(S_lambda_lists[i], B_lambda_teta_list, T_O2_list,
                         T_O3_list, T_H2O_list, mu_0, albedo, tau_0_a, beta, g, tau_m, list);
    EQ.push_back(compute_eq(B1, B2, albedo, dividers[i], dark_pixels[i]));
  }
  return EQ;
}

PYBIND11_MODULE(catlib, m) {

  m.doc() = "CAT functions";

  m.def("compute_tau_m",      &compute_tau_m,      "A function which compute tau_m array");
  m.def("compute_x_m",        &compute_x_m,        "A function which compute x_m value");
  m.def("compute_x_a",        &compute_x_a,        "A function which compute x_a value");
  m.def("compute_tau_a",      &compute_tau_a,      "A function which compute tau_a array");
  m.def("compute_tau_lambda", &compute_tau_lambda, "A function which compute tau_lambda array");
  m.def("compute_omega",      &compute_omega,      "A function which compute tau_lambda array");
  m.def("compute_g",          &compute_g,          "A function which compute g_lambda array");
  m.def("compute_x",          &compute_x,          "A function which compute x array");
  m.def("compute_B_atm",      &compute_B_atm,      "A function which compute B atm array");
  m.def("compute_B1",         &compute_B1,         "A function which compute B1 value");
  m.def("compute_B2",         &compute_B2,         "A function which compute B2 value");
  m.def("compute_u",          &compute_u,          "A function which compute u value");
  m.def("compute_v",          &compute_v,          "A function which compute v value");
  m.def("compute_w",          &compute_w,          "A function which compute w value");
  m.def("compute__T_dif",     &compute_T_dif,      "A function which compute T_dif array");
  m.def("compute_T_lambda",   &compute_T_lambda,   "A function which compute T_lambda array");
  m.def("compute_eq",         &compute_eq,         "A function which compute eq value");
  m.def("compute_EQ",         &compute_EQ,         "A function which compute EQ array");
}
