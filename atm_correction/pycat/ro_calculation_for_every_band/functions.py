import numpy as np
from math import pi


def compute_tau_m(tau_m_0, lambda_0, lambda_list):
    tau_m = []
    for lambda_index in range(0, len(lambda_list)):
        lambda_0_lambda = lambda_0 / lambda_list[lambda_index]
        tau_m.append(tau_m_0 * lambda_0_lambda ** 4)
    return tau_m


def compute_tau_a(lambda_0, lambda_list, tau_0_a, beta):
    tau_a = []
    for lambda_index in range(0, len(lambda_list)):
        lambda_0_lambda = lambda_0 / lambda_list[lambda_index]
        tau_a.append(tau_0_a * lambda_0_lambda ** beta)
    return tau_a


def compute_tau_lambda(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta):
    tau_lmb = []
    tau_m = compute_tau_m(tau_m_0, lambda_0, lambda_list)
    tau_a = compute_tau_a(lambda_0, lambda_list, tau_0_a, beta)
    for lambda_index in range(0, len(lambda_list)):
        tau_lmb.append(tau_m[lambda_index] + tau_a[lambda_index] + tau_e)
    return tau_lmb


def compute_omega(tau_m_0, tau_e, lambda_0, lambda_list, tau_0_a, beta):
    tau_m = compute_tau_m(tau_m_0, lambda_0, lambda_list)
    tau_a = compute_tau_a(lambda_0, lambda_list, tau_0_a, beta)
    omega = []
    for lambda_index in range(0, len(lambda_list)):
        omega.append((tau_m[lambda_index] + tau_a[lambda_index]) / (tau_m[lambda_index] + tau_a[lambda_index] + tau_e))
    return omega


def compute_x_m(mu_0):
    return 3 * (1 + mu_0 ** 2) / 4


def compute_x_a(mu_0, g):
    return (1 - g ** 2) / (1 + g ** 2 + 2 * g * mu_0) ** 1.5


def compute_x(mu_0, lambda_0, lambda_list, tau_m_0, g, tau_0_a, beta):
    x_m = compute_x_m(mu_0)
    x_a = compute_x_a(mu_0, g)
    tau_m = compute_tau_m(tau_m_0, lambda_0, lambda_list)
    tau_a = compute_tau_a(lambda_0, lambda_list, tau_0_a, beta)
    x = []
    for lambda_index in range(0, len(lambda_list)):
        x.append((x_m * tau_m[lambda_index] + x_a * tau_a[lambda_index]) / (tau_m[lambda_index] + tau_a[lambda_index]))
    return x


def compute_B_atm(mu_0, tau_m_0, lambda_0, lambda_list, p, tau_0_a, beta, g, q, tau_e):
    B_atm = []
    tau_lambda = compute_tau_lambda(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta)
    omega_lambda = compute_omega(tau_m_0, tau_e, lambda_0, lambda_list, tau_0_a, beta)
    x = compute_x(mu_0, lambda_0, lambda_list, tau_m_0, g, tau_0_a, beta)
    for lambda_index in range(0, len(lambda_list)):
        b_atm = omega_lambda[lambda_index] * x[lambda_index] / (4.0 * (1.0 + mu_0)) * (
                    1.0 - np.exp(-tau_lambda[lambda_index] * (1.0 / mu_0 + 1.0))) * (
                            1.0 + q * (omega_lambda[lambda_index] * tau_lambda[lambda_index]) ** p)
        B_atm.append(b_atm)
    return B_atm


def compute_B1(T_O2_list, T_O3_list, T_H2O_list, lambda_list, S_lambda_list, B_lambda_teta_list, mu_0, tau_m_0,
               lambda_0, p, tau_0_a, beta, g, q, tau_e):
    B1 = 0.0
    B_atm = compute_B_atm(mu_0, tau_m_0, lambda_0, lambda_list, p, tau_0_a, beta, g, q, tau_e)
    for lambda_index in range(0, len(lambda_list)):
        T_g_lambda = T_O2_list[lambda_index] * T_O3_list[lambda_index] * T_H2O_list[lambda_index]
        S_lambda = S_lambda_list[lambda_index]
        B_sun = B_lambda_teta_list[lambda_index]
        b = B_atm[lambda_index]

        B1 += b * T_g_lambda * S_lambda * B_sun
    return B1


def compute_g(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta):
    tau_m = compute_tau_m(tau_m_0, lambda_0, lambda_list)
    tau_a = compute_tau_a(lambda_0, lambda_list, tau_0_a, beta)
    g_lmb = []
    for lambda_index in range(0, len(lambda_list)):
        g_lmb.append(g * tau_a[lambda_index] / (tau_m[lambda_index] + tau_a[lambda_index]))
    return g_lmb


def compute_E_lambda(mu_0, tau_m_0, lambda_0, lambda_list, albedo, tau_0_a, beta, g, tau_e):
    tau_lambda = compute_tau_lambda(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta)
    omega_lambda = compute_omega(tau_m_0, tau_e, lambda_0, lambda_list, tau_0_a, beta)
    g_lmb = compute_g(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)

    E = []
    for lambda_index in range(0, len(lambda_list)):
        E_lmb = 4.0 * pi * omega_lambda[lambda_index] * mu_0 / (
                    4.0 + 3.0 * (1.0 - g_lmb[lambda_index]) * (1.0 - albedo) * tau_lambda[lambda_index]) * (
                            (0.5 + 0.75 * mu_0) + (0.5 - 0.75 * mu_0) * np.exp(-tau_lambda[lambda_index] / mu_0)) + (
                            1.0 - omega_lambda[lambda_index]) * pi * mu_0 * np.exp(-tau_lambda[lambda_index] / mu_0)
        E.append(E_lmb)
    return E


def compute_u(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta):
    g_lmb = compute_g(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)
    u = []
    for lambda_index in range(0, len(lambda_list)):
        h0 = -1.88227 + 0.53661 * g_lmb[lambda_index] - 1.8047 * g_lmb[lambda_index] ** 2 + 3.26348 * g_lmb[
            lambda_index] ** 3 - 2.3 * g_lmb[lambda_index] ** 4
        h1 = 5.97763 - 2.04621 * g_lmb[lambda_index] - 2.0173 * g_lmb[lambda_index] ** 2 + 1.44843 * g_lmb[
            lambda_index] ** 3
        h2 = -5.47825 + 2.42154 * g_lmb[lambda_index] - 3.37057 * g_lmb[lambda_index] ** 2 + 6.13805 * g_lmb[
            lambda_index] ** 3
        h3 = 2.07593 - 2.03761 * g_lmb[lambda_index] + 6.25975 * g_lmb[lambda_index] ** 2 - 7.35503 * g_lmb[
            lambda_index] ** 3
        u.append(h0 + h1 + h2 + h3)
    return u


def compute_v(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta):
    g_lmb = compute_g(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)
    v = []
    for lambda_index in range(0, len(lambda_list)):
        ro_0 = 0.4923 + 1.0471 * g_lmb[lambda_index] - 2.61112 * g_lmb[lambda_index] ** 2 + 1.53155 * g_lmb[
            lambda_index] ** 3
        ro_1 = 4.01521 - 0.25886 * g_lmb[lambda_index] - 2.85378 * g_lmb[lambda_index] ** 2 + 3.61515 * g_lmb[
            lambda_index] ** 3
        ro_2 = 3.76447 + 3.29106 * g_lmb[lambda_index] - 12.37951 * g_lmb[lambda_index] ** 2 + 9.85 * g_lmb[
            lambda_index] ** 3
        v.append(ro_0 + ro_1 * np.exp(-ro_2))
    return v


def compute_w(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta):
    g_lmb = compute_g(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)
    w = []
    for lambda_index in range(0, len(lambda_list)):
        q_0 = 0.000076 - 0.316 * g_lmb[lambda_index] + 0.67744 * g_lmb[lambda_index] ** 2 - 0.4093 * g_lmb[
            lambda_index] ** 3
        q_1 = -1.31136 - 0.8901 * g_lmb[lambda_index] + 3.55 * g_lmb[lambda_index] ** 2 - 3.0646 * g_lmb[
            lambda_index] ** 3
        q_2 = 5.21931 + 7.2255 * g_lmb[lambda_index] - 23.43878 * g_lmb[lambda_index] ** 2 + 17.65629 * g_lmb[
            lambda_index] ** 3
        w.append(q_0 + q_1 * np.exp(-q_2))
    return w


def compute_T_dif(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta, g):
    tau_lambda = compute_tau_lambda(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta)
    u = compute_u(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)
    v = compute_v(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)
    w = compute_w(tau_m_0, lambda_0, lambda_list, g, tau_0_a, beta)
    T_dif = []
    for lambda_index in range(0, len(lambda_list)):
        T_dif.append(tau_lambda[lambda_index] * np.exp(
            -u[lambda_index] - v[lambda_index] * tau_lambda[lambda_index] - w[lambda_index] * tau_lambda[lambda_index] *
            tau_lambda[lambda_index]))
    return T_dif


def compute_T_lambda(tau_m_0, lambda_0, lambda_list, tau_0_a, beta, g, tau_e):
    tau_lambda = compute_tau_lambda(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta)
    T_dif = compute_T_dif(tau_m_0, lambda_0, lambda_list, tau_e, tau_0_a, beta, g)
    T_lmb = []
    for lambda_index in range(0, len(lambda_list)):
        T_lmb.append((np.exp(-tau_lambda[lambda_index])) + T_dif[lambda_index])
    return T_lmb


def compute_B2(lambda_list, S_lambda_list, B_lambda_teta_list, T_O2_list, T_O3_list, T_H2O_list, mu_0, tau_m_0,
               lambda_0, albedo, tau_0_a, beta, g, tau_e):
    E_lambda = compute_E_lambda(mu_0, tau_m_0, lambda_0, lambda_list, albedo, tau_0_a, beta, g, tau_e)
    T_lambda = compute_T_lambda(tau_m_0, lambda_0, lambda_list, tau_0_a, beta, g, tau_e)

    B2 = 0.0
    for lambda_index in range(0, len(lambda_list)):
        T_g_lambda = T_O2_list[lambda_index] * T_O3_list[lambda_index] * T_H2O_list[lambda_index]
        S_lambda = S_lambda_list[lambda_index]
        B_sun = B_lambda_teta_list[lambda_index]
        T = T_lambda[lambda_index]
        E = E_lambda[lambda_index]
        B2 += E * T * T_g_lambda * S_lambda * B_sun
    return B2


def generate_range(low, high, count):
    return [low + x * (high - low) / (count - 1) for x in range(0, count)]
