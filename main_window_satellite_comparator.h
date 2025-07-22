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
#include "layer_list.h"

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
    void change_bands_sentinel_and_show_image();
    void show_layer(const QString id);
    void hide_layer(const QString id);
    void remove_scene_layer(const QString& id);
    void openLandsat9HeaderData();
    void openLandsat8HeaderData();
    void openSentinel2AHeaderData();
    void openCommonLandsatHeaderData(const QString& satellite_name);
    void openCommonSentinelHeaderData(const QString& satellite_name);
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
    inline QVector<double> getLandsat8Ksy(const int x, const int y);

    QString m_root_path;
    QImage m_satellite_image;
    uint16_t* m_landsat9_data_bands[LANDSAT_BANDS_NUMBER] = {nullptr};
    QPair<int,int> m_landsat9_bands_image_sizes[LANDSAT_BANDS_NUMBER];
    bool m_landsat9_missed_channels[LANDSAT_BANDS_NUMBER];



    double m_radiance_mult_add_arrays[LANDSAT_BANDS_NUMBER][2];
    double m_reflectance_mult_add_arrays[LANDSAT_BANDS_NUMBER][2];
    double m_lattitude;
    double m_longitude;

    bool m_is_image_created;
    bool m_is_bekas;
    LayerList* m_layer_gui_list;
    QCustomPlot* preview;
    QComboBox* calculation_method;
    QVector<double> m_landsat9_sample;
    QVector<double> m_sentinel_sample;
    QVector<double> m_bekas_sample;
    QGraphicsPixmapItem* m_image_item = nullptr;
    QHash<const QString,QGraphicsPixmapItem*> m_layer_items;
    CrossSquare *cross_square;
    QDoubleSpinBox* euclid_param_spinbox;
    QGraphicsTextItem* qgti;
    UasvViewWindow* bekas_window;
    void paintSamplePoints(const QColor &color);
    QString getGeoCoordinates(const int x, const int y);



    inline double euclideanDistance(const QVector<double>& v1,
                                    const QVector<double>& v2);

    inline double calculateSpectralAngle(const QVector<double>& S1,
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

    void processLayer(uchar* layer,
                      int xSize,
                      int yStart,
                      int yEnd,
                      const QVector<double> sample,
                      QColor color,
                      int offsetStart);


    sad::SENTINEL_METADATA m_sentinel_metadata;



    void initSentinelStructs();

    QVector<sad::BAND_DATA> m_sentinel_data;
    void read_sentinel2_bands_data();

    void gdal_start_driver();
    void gdal_close_driver();

    QPair<QVector<double>,QVector<double>> getSentinelKsy(const int x, const int y);

    void clear_satellite_data();
    void clear_all_layers();

    QHash <QString,geoTransform> sentinel_geo;
    QHash<QString, geoTransform> extractGeoPositions(const QString& xmlFilePath);
    int extractUTMZoneFromXML(const QString& xmlFilePath);


};
#endif // MAIN_WINDOW_SATELLITE_COMPARATOR_H
