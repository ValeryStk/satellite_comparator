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
#include "udpjsonrpc.h"

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
    void show_layer(const QString& id);
    void hide_layer(const QString& id);
    void remove_scene_layer(const QString& id);
    void add_roi_to_gui_list(const QString& id);
    void show_roi_average(const QString& id);

    void openLandsat9HeaderData();
    void openLandsat8HeaderData();
    void openSentinel2AHeaderData();
    void openSentinel2BHeaderData();
    void openBekasSpectraData();
    void findAreasUsingSelectedMetric();
    void centerSceneOnCrossSquare();

    void cursorPointOnSceneChangedEvent(QPointF pos);
    void samplePointOnSceneChangedEvent(QPointF pos);

    void openCommonLandsatHeaderData(const QString& satellite_name);
    void openCommonSentinelHeaderData(const QString& satellite_name);
    void processBekasDataForComparing(const QVector<double>& x,
                                      const QVector<double>& y);
    void showGoogleMap();
    void resetColorsToDefaultRGB();
    void handleJsonRpcResult(const QJsonValue &result);

    void processTestMatlabRequest(const QVariantMap &params);
    void processpClassifiedBecasSpectraMatlabRequest(const QVariantMap &params);
    void updateImage();


private:
    Ui::MainWindowSatelliteComparator* ui;
    QLabel* m_label_scene_coord;
    QGraphicsScene* m_scene;
    QGraphicsPixmapItem* m_image_item = nullptr;
    QHash<const QString,QGraphicsPixmapItem*>  m_layers_search_result_items;
    QHash<const QString,QGraphicsPolygonItem*> m_layers_roi_items;
    CrossSquare* m_scene_cross_square_item;


    sad::SATELLITE_TYPE m_satelite_type = sad::UNKNOWN_SATELLITE;
    DynamicCheckboxWidget* m_dynamic_checkboxes_widget;
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
    LayerList* m_layer_roi_list;
    QCustomPlot* m_preview_plot;
    QComboBox* m_comboBox_calculation_method;
    QVector<double> m_landsat9_sample;
    QVector<double> m_sentinel_sample;
    QVector<double> m_bekas_sample;



    QDoubleSpinBox* euclid_param_spinbox;
    QGraphicsTextItem* m_scene_text_item_metric_value;
    UasvViewWindow* bekas_window;
    void paintSamplePoints(const QColor &color);
    QString getGeoCoordinates(const int x, const int y);



    inline double euclideanDistance(const QVector<double>& v1,
                                    const QVector<double>& v2);

    inline double calculateSpectralAngle(const QVector<double>& S1,
                                         const QVector<double>& S2);


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
    void initLandsatStructs();
    void setUpPreviewPlot();
    void setUpToolWidget();
    void makeConnectsForMenuActions();
    void addBaseItemsToScene();

    QVector<sad::BAND_DATA> m_sentinel_data;
    QVector<sad::BAND_DATA> m_landsat_data;
    void read_sentinel2_bands_data();

    void gdal_start_driver();
    void gdal_close_driver();

    QPair<QVector<double>,QVector<double>> getSentinelKsy(const int x, const int y);

    void clear_satellite_data();
    void clear_all_layers();

    QHash <QString,geoTransform> sentinel_geo;
    QHash<QString, geoTransform> extractGeoPositions(const QString& xmlFilePath);
    int extractUTMZoneFromXML(const QString& xmlFilePath);
    void getKSY(const QPointF& pos, QVector<double>& waves, QVector<double>& ksy);
    QImage createModifiedImage(const QImage &img, double coefSat, double coefLight);
    void initUdpRpcConnection();
    UdpJsonRpc   *m_rpc;


};
#endif // MAIN_WINDOW_SATELLITE_COMPARATOR_H
