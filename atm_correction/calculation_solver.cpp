#include "calculation_solver.h"
#include "atm_correction.cpp"

calculation_solver::calculation_solver() {
  ::loadAllLists();
}

QStringList calculation_solver::getSatellitesList() {
  return lss::getSatellitesList();
}

void calculation_solver::updateCurrentSatellite(QString sat_name) {
  lss::updateSatelliteResponses(sat_name);
}

double calculation_solver::calculateAlbedo(double tau,
                                           double beta,
                                           double g,
                                           int band_number,
                                           double band_value) {
  return lss::calculateAlbedo(tau,
                              beta,
                              g,
                              band_number,
                              band_value);
}

void calculation_solver::setElavationAngle(double angle) {
  lss::setElevationAngle(angle);
}

void calculation_solver::solve_dark_pixels(const QString& satellite_name,
                                           const QVector<double>& dark_pixels) {
  qDebug() << "----------SOLVE DARK PIXEL------------------------";
  if (dark_pixels.size() < 4)
    return;
  qDebug() << "Solve dark pixel for satellite: " << satellite_name;
  auto result = lss::optimize(satellite_name, dark_pixels);
  emit darkpixels_calculation_finished(result);
}
