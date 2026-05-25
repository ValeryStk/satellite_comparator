#ifndef CALCULATION_SOLVER_H
#define CALCULATION_SOLVER_H

#include <QObject>
#include <QVector>

#include "common_types.h"

class calculation_solver : public QObject {
    Q_OBJECT
    friend class atm_correction_UnitTests;

public:
    calculation_solver();
    std::vector<double> getLambdaList() const;
    std::vector<std::vector<double> > getResponsesList();
    std::vector<double> get_h2o();
    std::vector<double> get_o3();
    std::vector<double> get_a_H2O();
    std::vector<double> get_b_H2O();
    double get_mH2O(double B9, double B8A);
    double get_TO3(int band_index, int X);
    void updateCurrentSatellite(QString sat_name);
    static double calculateAlbedo(double tau, double beta, double g,
                                  int band_number, double band_value);
    void start_solve_dark_pixels_async(const QString& satellite_name,
                                       const QVector<double>& dark_pixels);
    std::vector<double> loadDoublesFromFile(const QString& filePath);
public slots:
    void setSunZenitAngle(double angle);
    void solve_dark_pixels(const QString& satellite_name,
                           const QVector<double>& dark_pixels);
signals:
    void darkpixels_calculation_finished(result_values);
};
double calculateAlbedo(double tau, double beta, double g, int band_number,
                       double band_value);
#endif  // CALCULATION_SOLVER_H
