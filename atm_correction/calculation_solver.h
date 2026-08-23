#ifndef CALCULATION_SOLVER_H
#define CALCULATION_SOLVER_H

#include <QObject>
#include <QVector>

#include "common_types.h"

class calculation_solver : public QObject {
    Q_OBJECT
    friend class atm_correction_UnitTests;

public:
    calculation_solver(QVector<double> initial_values);
    std::vector<double> getLambdaList() const;
    std::vector<std::vector<double> > getResponsesList();
    std::vector<double> get_h2o();
    std::vector<double> get_o3();
    std::vector<double> get_a_H2O();
    std::vector<double> get_b_H2O();
    double get_mH2O(double B9, double B8A);
    double get_TO3(int band_index, int X);
    void updateCurrentSatellite(QString sat_name);
    static QVector<double> calculateAlbedo(QVector<double> speya_values,
                                           int classNum = 0);
    void start_solve_dark_pixels_async(const QString& satellite_name,
                                       const QVector<double>& dark_pixels);
    std::vector<double> loadDoublesFromFile(const QString& filePath);
    void setH2O(QVector<double> new_h20_list);
    void setInitial_values(const QVector<double>& initial_values);

public slots:
    void setSunZenitAngle(double angle);
    void setCaptruretZenitAngle(double angle);
    void setFiAngle(double angleSA, double angleCA);
    void computeGamma();
    void solve_dark_pixels(const QString& satellite_name,
                           const QVector<double>& dark_pixels);
signals:
    void darkpixels_calculation_finished(result_values);

private:
    QVector<double> m_initial_values;
};
double calculateAlbedo(double tau, double beta, double g, int band_number,
                       double band_value);
#endif  // CALCULATION_SOLVER_H
