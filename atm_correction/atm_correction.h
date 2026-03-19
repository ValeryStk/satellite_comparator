#ifndef ATM_CORRECTION
#define ATM_CORRECTION

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <vector>

using std::vector;
struct ro_final {
    double B1 = 0.0;
    double band_number = 0.0;
    double band_value = 0.0;
} ro_fin;
vector<vector<double>> S_lambda_lists(
    4);  // TODO adopt this container for 5 channels
vector<double> lambda_waves;
vector<double> T_H2O_list;
vector<double> lambda_list;
vector<double> T_O2_list;
vector<double> T_O3_list;
vector<double> B_lambda_teta_list;
vector<double> divider_list;
vector<double> tau_m;
QJsonObject satellites;
QJsonArray sdb;
QString satellite_name_key = "bka";
QStringList satellites_list;
vector<double> dark_pixels = {39.535587, 25.645323, 11.881793, 4.310712};

#endif  // ENVIMODULE_H
