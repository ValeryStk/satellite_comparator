#ifndef CALCULATION_SOLVER_H
#define CALCULATION_SOLVER_H

#include <QObject>
#include <QVector>

#include "common_types.h"
#include "json_utils.h"

class calculation_solver : public QObject {
    Q_OBJECT
    friend class atm_correction_UnitTests;

public:
    calculation_solver();
    QStringList getSatellitesList();
    void updateCurrentSatellite(QString sat_name);
    static double calculateAlbedo(double tau, double beta, double g,
                                  int band_number, double band_value);
    void start_solve_dark_pixels_async(const QString& satellite_name,
                                       const QVector<double>& dark_pixels);

public slots:
    void setElavationAngle(double angle);
    void solve_dark_pixels(const QString& satellite_name,
                           const QVector<double>& dark_pixels);
signals:
    void darkpixels_calculation_finished(result_values);
};
double calculateAlbedo(double tau, double beta, double g, int band_number,
                       double band_value);
#endif  // CALCULATION_SOLVER_H
