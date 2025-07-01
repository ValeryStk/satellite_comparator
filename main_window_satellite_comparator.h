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
#include "satellites_structs.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowSatelliteComparator; }
QT_END_NAMESPACE



class MainWindowSatelliteComparator : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowSatelliteComparator(QWidget *parent = nullptr);
    ~MainWindowSatelliteComparator();

private slots:
    void change_bands_and_show_image();
    void openLandsat9HeaderData();
    void openLandsat8HeaderData();
    void openCommonLandsatHeaderData(const QString& satellite_name);
    void processBekasDataForComparing(const QVector<double>& x,
                                      const QVector<double>& y);

private:
    Ui::MainWindowSatelliteComparator *ui;
    QGraphicsScene *scene;
    sad::SATELLITE_TYPE m_satelite_type = sad::UKNOWN_SATELLITE;
    DynamicCheckboxWidget *m_dynamic_checkboxes_widget;
    SatteliteComparator* m_sat_comparator;

    QStringList getLandSat9BandsFromTxtFormat(const QString& path,
                                              QList<QString> &available_gui_bands);
    QString getLandSatSpaceCraftIDFromTxtFormat(const QString& path);
    void fillLandSat9ReflectanceMultAdd(const QString& path);
    void fillLandSat9GeoData(const QString& path);
    void clearLandsat9DataBands();
    uchar* m_image_data;
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
