#ifndef MAIN_WINDOW_SATELLITE_COMPARATOR_H
#define MAIN_WINDOW_SATELLITE_COMPARATOR_H

#include <QMainWindow>
#include <cstdint>
#include <sattelite_comparator.h>
#include "dynamic_checkbox_widget.h"
#include "satellite_graphics_view.h"
#include "cross_square.h"
#include "QDoubleSpinBox"
#include "QComboBox"
#include "bekas/GuiModules/UasvViewWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowSatelliteComparator; }
QT_END_NAMESPACE

#define LANDSAT_9_BANDS_NUMBER 11

class MainWindowSatelliteComparator : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowSatelliteComparator(QWidget *parent = nullptr);
    ~MainWindowSatelliteComparator();

private slots:
    void change_bands_and_show_image();
    void openHeaderData();
    void processBekasDataForComparing(const QVector<double>& x,
                                      const QVector<double>& y);

private:
    Ui::MainWindowSatelliteComparator *ui;
    DynamicCheckboxWidget *m_dynamic_checkboxes_widget;
    SatteliteComparator* m_sat_comparator;

    QStringList getLandSat9BandsFromTxtFormat(const QString& path,
                                              QList<QString> &available_gui_bands);
    void fillLandSat9radianceMultAdd(const QString& path);
    void clearLandsat9DataBands();
    uint16_t* readTiff(const QString& path,
                       int& xSize,
                       int& ySize);

    void read_landsat_bands_data(const QStringList& file_names);
    QVector<double> getLandsat8Speya(const int x, const int y);
    QVector<double> getLandsat8Ksy(const int x, const int y);

    QString m_root_path;
    QImage m_satellite_image;
    uint16_t* m_landsat9_data_bands[LANDSAT_9_BANDS_NUMBER] = {nullptr};
    QPair<int,int> m_landsat9_bands_image_sizes[LANDSAT_9_BANDS_NUMBER];
    bool m_landsat9_missed_channels[LANDSAT_9_BANDS_NUMBER];
    const QString m_landsat9_bands_keys[LANDSAT_9_BANDS_NUMBER] = {
        "FILE_NAME_BAND_1",
        "FILE_NAME_BAND_2",
        "FILE_NAME_BAND_3",
        "FILE_NAME_BAND_4",
        "FILE_NAME_BAND_5",
        "FILE_NAME_BAND_6",
        "FILE_NAME_BAND_7",
        "FILE_NAME_BAND_8",
        "FILE_NAME_BAND_9",
        "FILE_NAME_BAND_10",
        "FILE_NAME_BAND_11"
    };
    const QString m_landsat9_mult_radiance_keys[LANDSAT_9_BANDS_NUMBER] = {
        "RADIANCE_MULT_BAND_1",
        "RADIANCE_MULT_BAND_2",
        "RADIANCE_MULT_BAND_3",
        "RADIANCE_MULT_BAND_4",
        "RADIANCE_MULT_BAND_5",
        "RADIANCE_MULT_BAND_6",
        "RADIANCE_MULT_BAND_7",
        "RADIANCE_MULT_BAND_8",
        "RADIANCE_MULT_BAND_9",
        "RADIANCE_MULT_BAND_10",
        "RADIANCE_MULT_BAND_11"
    };
    const QString m_landsat9_add_radiance_keys[LANDSAT_9_BANDS_NUMBER] = {
        "RADIANCE_ADD_BAND_1",
        "RADIANCE_ADD_BAND_2",
        "RADIANCE_ADD_BAND_3",
        "RADIANCE_ADD_BAND_4",
        "RADIANCE_ADD_BAND_5",
        "RADIANCE_ADD_BAND_6",
        "RADIANCE_ADD_BAND_7",
        "RADIANCE_ADD_BAND_8",
        "RADIANCE_ADD_BAND_9",
        "RADIANCE_ADD_BAND_10",
        "RADIANCE_ADD_BAND_11"
    };

    const QString m_landsat9_mult_reflectence_keys[LANDSAT_9_BANDS_NUMBER] = {
        "REFLECTANCE_MULT_BAND_1",
        "REFLECTANCE_MULT_BAND_2",
        "REFLECTANCE_MULT_BAND_3",
        "REFLECTANCE_MULT_BAND_4",
        "REFLECTANCE_MULT_BAND_5",
        "REFLECTANCE_MULT_BAND_6",
        "REFLECTANCE_MULT_BAND_7",
        "REFLECTANCE_MULT_BAND_8",
        "REFLECTANCE_MULT_BAND_9",
        "REFLECTANCE_MULT_BAND_10",
        "REFLECTANCE_MULT_BAND_11"
    };

    const QString m_landsat9_add_reflectence_keys[LANDSAT_9_BANDS_NUMBER] = {
        "REFLECTANCE_ADD_BAND_1",
        "REFLECTANCE_ADD_BAND_2",
        "REFLECTANCE_ADD_BAND_3",
        "REFLECTANCE_ADD_BAND_4",
        "REFLECTANCE_ADD_BAND_5",
        "REFLECTANCE_ADD_BAND_6",
        "REFLECTANCE_ADD_BAND_7",
        "REFLECTANCE_ADD_BAND_8",
        "REFLECTANCE_ADD_BAND_9",
        "REFLECTANCE_ADD_BAND_10",
        "REFLECTANCE_ADD_BAND_11"
    };

    const QString m_landsat9_bands_gui_names[LANDSAT_9_BANDS_NUMBER] = {
        "443 nm (Aerosol) 30 m",
        "482 nm (Blue) 30 m",
        "562 nm (Green) 30 m",
        "655 nm (Red) 30 m",
        "865 nm (Near infrared) 30 m",
        "1610 nm (SWIR 1) 30 m",
        "2200 nm (SWIR 2) 30 m",
        "590 nm (Panchromatic) 15 m",
        "1375 nm (Cirrus) 30 m",
        "10800 nm (LWIR) 100 m",
        "12000 nm (LWIR) 100 m"
    };

    double m_radiance_mult_add_arrays[LANDSAT_9_BANDS_NUMBER][2];
    double m_reflectance_mult_add_arrays[LANDSAT_9_BANDS_NUMBER][2];
    double m_lattitude;
    double m_longitude;

    bool m_is_image_created;
    bool m_is_bekas;
    QCustomPlot* preview;
    QComboBox* calculation_method;
    QVector<double> m_landsat9_sample;
    QVector<double> m_bekas_sample;
    QGraphicsPixmapItem* m_image_item = nullptr;
    CrossSquare *cross_square;
    QDoubleSpinBox* euclid_param_spinbox;
    QGraphicsTextItem* qgti;
    UasvViewWindow* bekas_window;
    void paintSamplePoints(const QColor &color);
    QString getGeoCoordinates(const int x, const int y);



    double euclideanDistance(const QVector<double>& v1,
                             const QVector<double>& v2);

    double calculateSpectralAngle(const QVector<double>& S1,
                                  const QVector<double>& S2);

    void showGoogleMap();


    struct geoTransform {
        double ulX = 0;           // Верхний левый X (восточное направление)
        double resX = 0;          // Разрешение по X
        double rotateX = 0;       // Поворот X (обычно 0 для Landsat)
        double ulY = 0;           // Верхний левый Y (северное направление)
        double rotateY = 0;       // Поворот Y (обычно 0 для Landsat)
        double resY = 0;          // Разрешение по Y (отрицательное, т.к. ось Y направлена вниз)
        double utmZone = 0;
    } m_geo;

};
#endif // MAIN_WINDOW_SATELLITE_COMPARATOR_H
