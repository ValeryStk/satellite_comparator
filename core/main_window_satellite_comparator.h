#ifndef MAIN_WINDOW_SATELLITE_COMPARATOR_H
#define MAIN_WINDOW_SATELLITE_COMPARATOR_H

#include <sattelite_comparator.h>

#include <QMainWindow>
#include <cstdint>

#include "QComboBox"
#include "QDoubleSpinBox"
#include "QtConcurrent/QtConcurrent"
#include "bekas/GuiModules/UasvViewWindow.h"
#include "cross_square.h"
#include "dynamic_checkbox_widget.h"
#include "layer_list.h"
#include "satellite_graphics_view.h"
#include "satellites_structs.h"
#include "spectral_indices_widget.h"
#include "udpjsonrpc.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindowSatelliteComparator;
}
QT_END_NAMESPACE

/*!
 * \brief Класс главного окна программы
 * Предназначен для отображения загруженных спутниковых данных,
 * управления поиском, визуализации градиентов усыхания для временного ряда,
 * отображения спектральных данных для текущего пикселя
 */

class MainWindowSatelliteComparator : public QMainWindow {
    Q_OBJECT

public:
    //! Конмтруктор
    explicit MainWindowSatelliteComparator(QWidget *parent = nullptr);

    //! Деструктор
    ~MainWindowSatelliteComparator();

private slots:
    //! Слот для изменения отображения каналов одиночного изображения
    void change_bands_and_show_image();

    //! Слот для изменения каналов для временного ряда (загружается базовое
    //! изображение - самый поздний момент времени)
    void change_bands_and_show_image(const QVector<sad::BAND_DATA> &band_data);

    //! Слот изменения каналов
    void change_bands();

    //! Слот отображения слоя
    //! \param id - идентификатор слоя
    void show_layer(const QString &id);

    //!
    //! \brief Слот сокрытия слоя
    //! \param id - идентификатор слоя
    //!
    void hide_layer(const QString &id);

    //!
    //! \brief Слот для удаления слоя со сцены
    //! \param id - идентификатор слоя
    //!
    void remove_scene_layer(const QString &id);

    //!
    //! \brief Слот для добавления региона интереса в список
    //! \param id - идентификатор региона интереса
    //!
    void add_roi_to_gui_list(const QString &id);

    //!
    //! \brief  Слот для расчёта среднего КСЯ по области
    //! \param id - идентификатор региона интереса
    //!
    void show_roi_average(const QString &id);

    //!
    //! \brief  Слот для отправки данных внутри региона интереса в матлаб
    //! \param id - идентификатор региона интереса
    //!
    void send_roi_spectrs_to_matlab(const QString &id);

    //!
    //! \brief Слот расчёта градиента усыхания
    //! \param id - идентификатор региона интереса
    //!
    void calculate_time_row_gradient(const QString &id);

    //! Слот для открытия данных Landsat 9
    void openLandsat9HeaderData();

    //! Слот для открытия данных Landsat 8
    void openLandsat8HeaderData();

    //! Слот для открытия данных Sentinel 2A
    void openSentinel2AHeaderData();

    //! Слот для открытия данных Sentinel 2B
    void openSentinel2BHeaderData();

    //! Слот для открытия данных Bekas
    void openBekasSpectraData();

    //! Слот для открытия временного ряда
    void openTimeRowData();

    //! Слот для нахождения области. Используется метрика выбранная
    //! пользователем.
    void findAreasUsingSelectedMetric();

    //! Центрирование сцены на пикселе, который находится в центре перекрестия
    //! выбора образца для поиска
    void centerSceneOnCrossSquare();

    //! \brief  Слот обработки события изменения положения курсора мыши на сцене
    //! \param pos - текущая точка под курсором
    void cursorPointOnSceneChangedEvent(QPointF pos);

    //! \brief Слот обработки события выбора образца для поиска
    //! \param pos - выбранная точка изображения
    void samplePointOnSceneChangedEvent(QPointF pos);

    //! \brief openCommonLandsatHeaderData
    //! \param satellite_name - имя спутника
    void openCommonLandsatHeaderData(const QString &satellite_name);

    //! \brief openCommonSentinelHeaderData
    //! \param satellite_name
    void openCommonSentinelHeaderData(const QString &satellite_name);

    //! \brief processBekasDataForComparing
    //! \param x
    //! \param y
    void processBekasDataForComparing(const QVector<double> &x,
                                      const QVector<double> &y);
    //!
    //! \brief showGoogleMap
    //!
    void showGoogleMap();

    //!
    //! \brief resetColorsToDefaultRGB
    //!
    void resetColorsToDefaultRGB();

    //!
    //! \brief handleJsonRpcResult
    //! \param result
    //!
    void handleJsonRpcResult(const QJsonValue &result);

    //!
    //! \brief processTestMatlabRequest
    //! \param params
    //!
    void processTestMatlabRequest(const QVariantMap &params);

    //!
    //! \brief processpClassifiedBecasSpectraMatlabRequest
    //! \param params
    //!
    void processpClassifiedBecasSpectraMatlabRequest(const QVariantMap &params);

    //!
    //! \brief processpClassifiedMultiSpecMatlabRequest
    //! \param params
    //!
    void processpClassifiedMultiSpecMatlabRequest(const QVariantMap &params);

    //!
    //! \brief updateImage
    //!
    void updateImage();

    void runChangeDetectionMethod();

private:
    //! \brief Указатель на графический интерфейс пользователя главного окна
    //! программы
    Ui::MainWindowSatelliteComparator *ui;

    //! \brief Виджет временного ряда
    QWidget m_time_row_widget;

    //! \brief Текстовое поле для оотображения географических координат
    QLabel *m_label_scene_coord;

    //! \brief Графическая сцена
    QGraphicsScene *m_scene;

    //! \brief Объект изображения графической сцены
    QGraphicsPixmapItem *m_image_item = nullptr;

    //!
    //! \brief m_layers_search_result_items
    //!
    QHash<const QString, QGraphicsPixmapItem *> m_layers_search_result_items;

    //!
    //! \brief m_layers_roi_items
    //!
    QHash<const QString, QGraphicsPolygonItem *> m_layers_roi_items;

    //!
    //! \brief m_scene_cross_square_item
    //!
    CrossSquare *m_scene_cross_square_item;

    //!
    //! \brief m_satelite_type
    //!
    sad::SATELLITE_TYPE m_satelite_type = sad::UNKNOWN_SATELLITE;

    //!
    //! \brief m_dynamic_checkboxes_widget
    //!
    DynamicCheckboxWidget *m_dynamic_checkboxes_widget;

    //!
    //! \brief m_sat_comparator
    //!
    SatteliteComparator *m_sat_comparator;

    //!
    //! \brief getLandSat9BandsFromTxtFormat
    //! \param path
    //! \param available_gui_bands
    //! \return
    //!
    QStringList getLandSat9BandsFromTxtFormat(
        const QString &path, QList<QString> &available_gui_bands);

    //!
    //! \brief getLandSatSpaceCraftIDFromTxtFormat
    //! \param path
    //! \return
    //!
    QString getLandSatSpaceCraftIDFromTxtFormat(const QString &path);

    //!
    //! \brief fillLandSat9ReflectanceMultAdd
    //! \param path
    //!
    void fillLandSat9ReflectanceMultAdd(const QString &path);

    //!
    //! \brief fillLandSat9GeoData
    //! \param path
    //!
    void fillLandSat9GeoData(const QString &path);

    //!
    //! \brief clearLandsat9DataBands
    //!
    void clearLandsat9DataBands();

    //!
    //! \brief cursorPointOnSceneChangedEventTimeRow
    //! \param pos
    //! \param is_landsat
    //!
    inline void cursorPointOnSceneChangedEventTimeRow(const QPointF &pos,
                                                      const bool is_landsat);

    //!
    //! \brief m_image_data
    //!
    uchar *m_image_data;

    //!
    //! \brief readTiff
    //! \param path
    //! \param xSize
    //! \param ySize
    //! \return
    //!
    uint16_t *readTiff(const QString &path, int &xSize, int &ySize);

    //!
    //! \brief read_landsat_bands_data
    //! \param file_names
    //!
    void read_landsat_bands_data(const QStringList &file_names);
    QVector<double> getLandsat8Speya(const int x, const int y);
    inline QVector<double> getLandsat8Ksy(const int x, const int y);

    QString m_root_path;  //!< Путьк открытой корневой директории
    QImage m_satellite_image;  //!< Базовое RGB изображение спутника
    uint16_t *m_landsat9_data_bands[LANDSAT_BANDS_NUMBER] = {
        nullptr};  //!< Данные каналов для каждого канала
    QPair<int, int> m_landsat9_bands_image_sizes
        [LANDSAT_BANDS_NUMBER];  //!< Размеры изображений для каждого канала
    bool
        m_landsat9_missed_channels[LANDSAT_BANDS_NUMBER];  //!< Не доступные для
                                                           //!< загрузки каналы

    double m_radiance_mult_add_arrays[LANDSAT_BANDS_NUMBER]
                                     [2];  //!< Коэффициенты привидения значений
                                           //!< АЦП в СПЭЯ
    double m_reflectance_mult_add_arrays[LANDSAT_BANDS_NUMBER]
                                        [2];  //!< Коэффициенты привидения
                                              //!< значений АЦП в КСЯ
    double m_lattitude;  //!< географическая широта для выбранного пикселя
    double m_longitude;  //!< географическая долгота для выбранного пикселя

    bool m_is_image_created;  //!< флаг созданного базового изображения
    bool m_is_bekas;  //!< флаг использования образца БЕКАС
    LayerList *m_layer_gui_list;  //!< Список базовых слоёв
    LayerList *m_layer_roi_list;  //!< Список областей интересов
    QCustomPlot *m_preview_plot;  //!< Объект для отображения графика КСЯ для
                                  //!< пикселя под курсором
    QComboBox *
        m_comboBox_calculation_method;  //!< Комбобокс для выбора метрики поиска
    QVector<double> m_landsat9_sample;  //!< Образец для поиска Landsat 9
    QVector<double> m_sentinel_sample;  //!< Образец для поиска Sentinel
    QVector<double> m_bekas_sample;  //!< Образец для поиска БЕКАС

    QDoubleSpinBox *euclid_param_spinbox;
    QGraphicsTextItem *m_scene_text_item_metric_value;
    UasvViewWindow *bekas_window;
    void paintSamplePoints(const QColor &color);
    QString getGeoCoordinates(const int x, const int y,
                              const sad::geoTransform &geo, double &latitude,
                              double &longitude, bool isStringReturn);
    QPointF geoToPixel(double latitude, double longitude,
                       const sad::geoTransform &gt);

    inline double euclideanDistance(const QVector<double> &v1,
                                    const QVector<double> &v2);

    inline double calculateSpectralAngle(const QVector<double> &S1,
                                         const QVector<double> &S2);

    sad::geoTransform m_geo;

    void processLayer(uchar *layer, int xSize, int yStart, int yEnd,
                      const QVector<double> sample, QColor color,
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
    void read_sentinel2_bands_data(QVector<sad::BAND_DATA> &data);

    void gdal_start_driver();
    void gdal_close_driver();

    QPair<QVector<double>, QVector<double>> getSentinelKsy(const int x,
                                                           const int y);
    //! \brief получение длин волн для данных Сентинел
    QVector<double> getSentinelWaves();

    //! \brief получение значений КСЯ для Сентинел
    //! \param x, y - координаты пикселя
    QVector<double> getSentinelKsyValues(const int x, const int y);

    //! \brief получение значений КСЯ для любого спутника
    //! \param x
    //! \param y
    QVector<double> getKsyValues(const int x, const int y);

    //! \brief получение длин волн для любого спутника
    QVector<double> getWaves();

    void clear_satellite_data();
    void clear_all_layers();

    QHash<QString, sad::geoTransform> extractGeoPositions(
        const QString &xmlFilePath);
    int extractUTMZoneFromXML(const QString &xmlFilePath);
    QString getDateTimeFromXML(const QString &xmlFilePath);
    void getKSY(const QPointF &pos, QVector<double> &waves,
                QVector<double> &ksy);
    QImage createModifiedImage(const QImage &img, double coefSat,
                               double coefLight);
    void initUdpRpcConnection();
    UdpJsonRpc *m_rpc;

    QVector<sad::BAND_DATA> getDataFromJsonForLandsat8_9_TimeRow(
        const QString &headerName, sad::LANDSAT_METADATA_FILE &landsat_metadata,
        sad::geoTransform &gt);

    QVector<sad::BAND_DATA> getDataForSentinel_TimeRow(
        const QString &headerName, sad::SATELLITE_TYPE st,
        sad::SENTINEL_METADATA &metadata, sad::geoTransform &gt);

    QVector<QVector<sad::BAND_DATA>> m_time_row;
    QVector<sad::QA_MASK_DATA> m_time_row_qa_mask;
    QVector<sad::geoTransform> m_time_row_geo;
    sad::geoTransform getGeo(const QJsonObject &jo);

    QVector<QImage> get_cropedImages_for_time_row(
        const QVector<QVector<sad::BAND_DATA>> &m_time_row,
        sad::SATELLITE_TYPE st);
    QCustomPlot *time_row_indexes_plot;
    QPair<QVector<double>, QVector<QString>> m_time_row_dates_unix_time;
    void showTimeRowIndexesDataViaPlot(QVector<double> &&ndvis,
                                       QVector<double> &&ndwis);
    bool isDataCloudShadow_OK(QVector<QPointF> &points);
    void paintTimeRowBadForest(const QColor &color);

    sad::NDWI_NDVI_TIME_ROW getIndexesForTimeRow(
        const QVector<QPointF> &points);
    double calculate_slope(const QVector<double> &values);

    SpectralIndicesWidget *m_spectralWidget;
    QDockWidget *m_spectralDock;
    void setUpUi();

    QVector<sad::BAND_DATA> change_detection_data;
    sad::geoTransform change_detection_geo;
};
#endif  // MAIN_WINDOW_SATELLITE_COMPARATOR_H
