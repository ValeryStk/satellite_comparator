#ifndef SATTELITE_COMPARATOR_H
#define SATTELITE_COMPARATOR_H

#include <QMainWindow>
#include <algorithm>
#include <QJsonObject>

class QCustomPlot;

namespace Ui {
class SatteliteComparator;
}
//!
//! \brief Структура для хранения типов графиков
//!
//!
enum class plot_type{
    ORIGIN_DEVICE_SPECTR,
    INTERPOLATED_DEVICE_SPECTR,
    SATELLITE_SPECTR,
    //ADD NEW TYPE BEFORE
    PLOT_TYPE_SIZE
};

struct satellites_data{
    QString alias = "";
    QString satellite_name;
    QVector<double> central_waves;
    QVector<QVector<double>> bands;
    QVector<QVector<double>> responses;
};

struct data_to_show{
    QVector<double> device_waves;
    QVector<double> device_values;
    QVector<double> satellite_waves;
    QVector<double> satellite_values;
    QVector<double> device_interpolated_values;
    QVector<double> device_interpolated_waves;
    QVector<double> folded_device_values;
};
//!
//! \brief SatteliteComparator class предназначен для сравнения двух спектров,
//! спектра спутникового сенсора и спектра какого-либо прибора
//!
class SatteliteComparator : public QMainWindow
{
    Q_OBJECT

public:
    explicit SatteliteComparator(QVector<double> device_waves = {},
                                 QVector<double> device_values = {},
                                 QVector<double> satellite_waves = {},
                                 QVector<double> satellite_values = {});
    ~SatteliteComparator();


private:
    Ui::SatteliteComparator *ui;
    QCustomPlot* m_plot;
    QJsonObject m_sdb;
    QStringList m_satellites_list;
    QHash<QString,satellites_data> m_all_satellites_data;
    QVector<double> m_common_wave_grid;
    satellites_data m_sat_data;
    data_to_show m_data_to_show;


    void addCharts();
    void loadJsonSatellitesCentralWaves();
    void base_check_before_interpolation(const QVector<double> &x,
                                         const QVector<double> &y);
    inline double linearInterpolation(const QVector<double> &x,
                                      const QVector<double> &y,
                                      const double target_x);


    QJsonObject get_sdb();
    QVector<double> get_common_waves();
    QVector<QVector<double>> get_bands(const QString &satellite_name);


    void compare_spectrs();
    void setSetSatelliteResponses(const QString& satellite_name);


public:
    QHash<QString,satellites_data> get_satellites_data();
    bool set_satellite_responses(const QString &satellite_name);

    int tryToFindTheSameVector(const QVector<QVector<double> > &vectorOfVectors,
                               const QVector<double> &targetVector);



   QPair<QVector<double>,QVector<double>> interpolate(const QVector<double> &x,
                                           const QVector<double> &y,
                                           const QVector<double> &new_x);

    void replot();
    void initial_fill_data_to_show(const QVector<double>& device_waves,
                                   const QVector<double>& device_values,
                                   const QVector<double>& satellite_waves,
                                   const QVector<double>& satellite_values);

    QVector<double> check_intersection(const QVector<double>&waves_1,
                                       const QVector<double>&waves_2);

    QVector<double> fold_spectr_to_satellite_responses();
};

#endif // SATTELITE_COMPARATOR_H
