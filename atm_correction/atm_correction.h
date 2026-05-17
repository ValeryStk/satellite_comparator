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

vector<vector<double>> S_lambda_lists;
vector<double> lambda_waves;
vector<double> T_H2O_list;
vector<double> lambda_list;
vector<double> T_O2_list;
vector<double> T_O3_list;
vector<double> B_lambda_teta_list;
vector<double> divider_list;
vector<double> tau_m;

QJsonArray atm_params;
QJsonArray sat_sentinel2A_respns;
QJsonArray sat_sentinel2B_respns;
QString satellite_name_key = "sentinel2a-10m";
QStringList satellites_list;
vector<double> dark_pixels = {39.535587, 25.645323, 11.881793, 4.310712};

#endif  // ENVIMODULE_H
