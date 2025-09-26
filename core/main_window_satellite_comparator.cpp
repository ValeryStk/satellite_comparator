#include "main_window_satellite_comparator.h"
#include "ui_main_window_satellite_comparator.h"

#include <algorithm>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include "libs/gdal/x64/include/gdal_priv.h"
#include "libs/gdal/x64/include/cpl_conv.h"
#include "libs/gdal/x64/include/cpl_string.h"
#include "libs/gdal/x64/include/ogr_spatialref.h"
#include "libs/gdal/x64/include/xtiffio.h"
#include "libs/gdal/x64/include/geotiff.h"
#include "libs/gdal/x64/include/geotiffio.h"
#include "libs/gdal/x64/include/tiff.h"
#include "libs/gdal/x64/include/tiffio.h"
#include "json_utils.h"
#include "QDebug"
#include "QImageReader"
#include "QCheckBox"
#include "QGraphicsPixmapItem"
#include "QGraphicsProxyWidget"
#include "qcustomplot.h"
#include "progress_informator.h"
#include "text_constants.h"
#include "google_maps_url_maker.h"
#include <QSpacerItem>
#include "satellite_xml_reader.h"
#include "layer_list.h"
#include "layer_roi_list.h"
#include "icon_generator.h"
#include "thread"
#include <QDomDocument>
#include "cpl_conv.h"
#include "QApplication"
#include <QByteArray>
#include <matio.h>
#include <MatFilesOperator.h>
#include "version.h"






constexpr int MAX_BYTES_IN_BASE_IMAGE_LAYER  = 11000 * 11000 * 3;


QCPTextElement *title_satellite_name;
QVector<double> waves_landsat9 = {443,482,562,655,865,1610,2200};
QVector<double> waves_landsat9_5 = {443,482,562,655,865};


// C++ слушает порт 5001, MATLAB — 5000
constexpr int LOCAL_PORT = 5001;
constexpr int MATLAB_PORT = 5000;

namespace {

void downsample_uint16(const uint16_t* input,
                       uint16_t* output,
                       const int width,
                       const int height) {

    int outWidth = width / 2;
    int outHeight = height / 2;

    for (int y = 0; y < outHeight; ++y) {
        for (int x = 0; x < outWidth; ++x) {
            // Среднее значение 2×2 пикселей
            int inX = x * 2;
            int inY = y * 2;

            uint32_t sum = 0;
            sum += input[inY * width + inX];
            sum += input[inY * width + (inX + 1)];
            sum += input[(inY + 1) * width + inX];
            sum += input[(inY + 1) * width + (inX + 1)];

            output[y * outWidth + x] = static_cast<uint16_t>(sum / 4);
        }
    }
}

void copyQStringArray(const QString source[], QString destination[], int size) {
    for (int i = 0; i < size; ++i) {
        destination[i] = source[i];
    }
}

}

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , m_scene(new QGraphicsScene)
    , m_scene_cross_square_item(new CrossSquare(100))
    , m_dynamic_checkboxes_widget(nullptr)
    , m_sat_comparator(new SatteliteComparator)
    , m_image_data(new uchar[MAX_BYTES_IN_BASE_IMAGE_LAYER])
    , m_is_image_created(false)
    , m_is_bekas(false)
    , m_scene_text_item_metric_value(new QGraphicsTextItem)
    , bekas_window(nullptr)


{
    ui->setupUi(this);
    m_label_scene_coord = new QLabel;
    ui->statusbar->addPermanentWidget(m_label_scene_coord);
    initUdpRpcConnection();
    QString app_title_version = "%1 %2 %3";
    setWindowTitle(app_title_version.arg(satc::app_name).arg(" ").arg(QString(VER_PRODUCTVERSION_STR)));
    gdal_start_driver();
    initSentinelStructs();
    initLandsatStructs();
    makeConnectsForMenuActions();
    addBaseItemsToScene();
    setUpPreviewPlot();
    connect(ui->graphicsView_satellite_image,SIGNAL(pointChanged(QPointF)),
            this,SLOT(cursorPointOnSceneChangedEvent(QPointF)));
    connect(ui->graphicsView_satellite_image,SIGNAL(sampleChanged(QPointF)),
            this,SLOT(samplePointOnSceneChangedEvent(QPointF)));
    ui->graphicsView_satellite_image->setUp(m_scene);
    setUpToolWidget();
    connect(ui->graphicsView_satellite_image,SIGNAL(roiPolygonAdded(const QString)),this,SLOT(add_roi_to_gui_list(const QString)));
    connect(ui->widget_image_saturation_light_corrector,SIGNAL(slidersWereChanged()), SLOT(updateImage()));
}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
    gdal_close_driver();
}

void MainWindowSatelliteComparator::openLandsat9HeaderData()
{
    m_satelite_type = sad::LANDSAT_9;
    openCommonLandsatHeaderData(satc::satellite_name_landsat_9);
}

void MainWindowSatelliteComparator::openLandsat8HeaderData()
{
    m_satelite_type = sad::LANDSAT_8;
    openCommonLandsatHeaderData(satc::satellite_name_landsat_8);
}

void MainWindowSatelliteComparator::openSentinel2AHeaderData()
{
    m_satelite_type = sad::SENTINEL_2A;
    openCommonSentinelHeaderData(satc::satellite_name_sentinel_2A);
}

void MainWindowSatelliteComparator::openSentinel2BHeaderData()
{
    m_satelite_type = sad::SENTINEL_2B;
    openCommonSentinelHeaderData(satc::satellite_name_sentinel_2B);
}

void MainWindowSatelliteComparator::openBekasSpectraData()
{
    bekas_window = new UasvViewWindow;
    bekas_window->setWindowTitle(satc::app_name);
    bekas_window->setAttribute(Qt::WA_DeleteOnClose);
    connect(bekas_window,SIGNAL(sendSampleForSatelliteComparator(QVector<double>, QVector<double>)),
            this,SLOT(processBekasDataForComparing(QVector<double>,QVector<double>)));
    bekas_window->setUDPobj(m_rpc);
    bekas_window->show();
}

void MainWindowSatelliteComparator::openTimeRowData()
{
    qDebug()<<"check time row connection....";
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку c временными рядами", QDir::homePath());
    QDir directory(dir);  // dir — путь, полученный из QFileDialog
    QStringList subdirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug()<<subdirs;
}

void MainWindowSatelliteComparator::findAreasUsingSelectedMetric()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
    QString message = QString("Пожалуйста подождите,\nпроисходит поиск областей\n(%1)...").arg(m_comboBox_calculation_method->currentText());
    ProgressInformator progress_info(ui->graphicsView_satellite_image,
                                     message);
    progress_info.show();
    QApplication::processEvents();
    paintSamplePoints(color);
    progress_info.close();
}

void MainWindowSatelliteComparator::centerSceneOnCrossSquare()
{
    ui->graphicsView_satellite_image->centerOn(m_scene_cross_square_item);
    ui->graphicsView_satellite_image->setTransform(QTransform());
}

void MainWindowSatelliteComparator::cursorPointOnSceneChangedEvent(QPointF pos)
{
    QVector<double> data;
    QVector<double> waves;
    QVector<double> sample;
    QVector<double> trimmed_satellite_data;

    const QString x_y = "x: %1   y:%2";
    QString x_y_message = x_y.arg(QString::number(pos.x()),QString::number(pos.y()));
    m_label_scene_coord->setText(x_y_message);

    if(m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_8||m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_9){
        data = getLandsat8Ksy(pos.x(),pos.y());
    }else if(m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2A||m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2B){
        auto w_k = getSentinelKsy(pos.x(),pos.y());
        data = w_k.second;
        waves = w_k.first;
        if(m_sentinel_sample.empty()){
            sample = data;//TEMPORARY
        }else{
            sample = m_sentinel_sample;
        }
        trimmed_satellite_data = data;
    }
    if(data.empty()){
        return;
    }

    if(m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_8||m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_9){

        if(data.size()!=(int)LANDSAT_BANDS_NUMBER-4){
            qDebug()<<"ERROR SIZE:"<<data.size();
            return;
        }
        if(m_is_bekas){
            sample = m_bekas_sample;
            waves = waves_landsat9_5;
            size_t elems_to_copy = std::min(static_cast<size_t>(data.size()), static_cast<size_t>(5));
            trimmed_satellite_data = data.mid(0,static_cast<int>(elems_to_copy));
        }else{
            sample = m_landsat9_sample;
            waves = waves_landsat9;
            trimmed_satellite_data = data;
        }
    }

    m_preview_plot->graph(0)->data().clear();
    m_preview_plot->graph(1)->data().clear();
    m_preview_plot->graph(0)->setData(waves, trimmed_satellite_data);
    m_preview_plot->graph(1)->setData(waves, sample);

    double result = 999;
    if(m_comboBox_calculation_method->currentText()==satc::spectral_angle){
        result = calculateSpectralAngle(trimmed_satellite_data,sample);
    }else if(m_comboBox_calculation_method->currentText()==satc::euclid_metrika){
        result = euclideanDistance(trimmed_satellite_data,sample);
    }
    m_scene_text_item_metric_value->setPos(pos.x(),pos.y()+5);
    m_scene_text_item_metric_value->setPlainText(QString::number(result));
    m_preview_plot->rescaleAxes(true);
    m_preview_plot->replot();
}

void MainWindowSatelliteComparator::samplePointOnSceneChangedEvent(QPointF pos)
{
    m_is_bekas = false;
    m_scene_cross_square_item->setPos(pos);
    m_scene_cross_square_item->update();
    QVector<double> data;
    QVector<double> sample;
    QVector<double> waves;
    if(m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_9||m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_8){
        data = getLandsat8Ksy(pos.x(),pos.y());

        if(data.empty())return;
        if(data.size()!=(int)LANDSAT_BANDS_NUMBER-4){
            qDebug()<<"ERROR SIZE:"<<data.size();
            return;
        }

        m_landsat9_sample = data;
        sample = m_landsat9_sample;
        waves = waves_landsat9;


    }else if(m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2A||m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2B){
        auto w_k = getSentinelKsy(pos.x(),pos.y());
        data = w_k.second;
        waves = w_k.first;
        m_sentinel_sample = data;
        sample = m_sentinel_sample;
    }


    m_preview_plot->graph(0)->data().clear();
    m_preview_plot->graph(1)->data().clear();
    m_preview_plot->graph(0)->setData(waves, data);
    m_preview_plot->graph(1)->setData(waves, sample);
    m_preview_plot->rescaleAxes(true);
    m_preview_plot->replot();

    getGeoCoordinates(pos.x(),pos.y());

}

void MainWindowSatelliteComparator::openCommonLandsatHeaderData(const QString& satellite_name)
{
    QString openSatMessage = QString("Открыть заголовочный файл %1").arg(satellite_name);
    QString headerName =  QFileDialog::getOpenFileName(this,openSatMessage,"",
                                                       "файлы(*_MTL.json *_MTL.txt *_MTL.xml)");
    ui->graphicsView_satellite_image->setIsSignal(false);
    clearLandsat9DataBands();
    clear_satellite_data();
    clear_all_layers();
    m_scene_cross_square_item->setVisible(false);
    QFile file(headerName);
    static bool isHeaderValid = false;
    if(file.exists()==false)return;
    if(m_dynamic_checkboxes_widget)m_dynamic_checkboxes_widget->clear();
    QFileInfo fi(headerName);
    m_root_path = fi.path();
    const QString extension = fi.completeSuffix();
    QString dataLoadingMessage = QString("Загрузка данных %1...").arg(satellite_name);
    ui->statusbar->showMessage(dataLoadingMessage);
    QApplication::processEvents();
    QList<QString> landsat_gui_available_bands;

    if(extension == "json"){
        QJsonObject jo;
        jsn::getJsonObjectFromFile(headerName,jo);
        if(jo.contains("LANDSAT_METADATA_FILE")){
            QJsonObject image_attributes = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","IMAGE_ATTRIBUTES"}).toObject();
            title_satellite_name->setText(image_attributes.value("SPACECRAFT_ID").toString());
            QJsonValue value = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","PRODUCT_CONTENTS"});
            QJsonValue radiance_value = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","LEVEL2_SURFACE_REFLECTANCE_PARAMETERS"});
            QJsonObject check_bands = value.toObject();
            QJsonObject radiance = radiance_value.toObject();
            QJsonObject projection = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","PROJECTION_ATTRIBUTES"}).toObject();
            m_geo.utmZone = projection["UTM_ZONE"].toString().toDouble();
            m_geo.ulX = projection["CORNER_UL_PROJECTION_X_PRODUCT"].toString().toDouble();
            m_geo.ulY = projection["CORNER_UL_PROJECTION_Y_PRODUCT"].toString().toDouble();
            m_geo.resX = projection["GRID_CELL_SIZE_REFLECTIVE"].toString().toDouble();
            m_geo.resY = - m_geo.resX;

            for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
                if(check_bands.value(sad::landsat9_bands_keys[i]).isUndefined()){
                    qDebug()<<"missed band: "<<sad::landsat9_bands_keys[i];
                    m_landsat9_missed_channels[i] = true;
                    continue;
                }
                m_landsat9_missed_channels[i] = false;
                landsat_gui_available_bands.append(sad::landsat_bands_gui_names[i]);
                auto band_file_name = check_bands[sad::landsat9_bands_keys[i]].toString();
                int xS;
                int yS;
                m_landsat9_data_bands[i] = readTiff(fi.path()+"/"+band_file_name,xS,yS);
                m_landsat9_bands_image_sizes[i] = {xS,yS};

                if(i<7){//TODO TEST Есть только до 9 канала TEST REFLECTANCE_MULT_BAND_9  REFLECTANCE_ADD_BAND_9
                    double mult_refl = radiance[sad::landsat9_mult_reflectence_keys[i]].toString().toDouble();
                    double add_refl = radiance[sad::landsat9_add_reflectence_keys[i]].toString().toDouble();
                    m_reflectance_mult_add_arrays[i][0] = mult_refl;
                    m_reflectance_mult_add_arrays[i][1] = add_refl;
                }
            }
            isHeaderValid = true;
        }else{
            return;// Пока работает только LANDSAT 9
        };
    }else if(extension == "txt"){
        auto file_names = getLandSat9BandsFromTxtFormat(headerName,
                                                        landsat_gui_available_bands);
        //qDebug()<<"TXT filenames: "<<file_names;
        title_satellite_name->setText(getLandSatSpaceCraftIDFromTxtFormat(headerName));
        read_landsat_bands_data(file_names);
        fillLandSat9ReflectanceMultAdd(headerName);
        fillLandSat9GeoData(headerName);
        isHeaderValid = true;
    }else if(extension == "xml"){
        auto data = satc::readLandsatXmlHeader(headerName);
        title_satellite_name->setText(data.image_attributes.spacecraft_id);
        QStringList file_names;
        for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
            if(data.landsat9_missed_channels[i])continue;
            file_names.append(data.product_contents.file_name_bands[i]);
            landsat_gui_available_bands.append(sad::landsat_bands_gui_names[i]);
            m_reflectance_mult_add_arrays[i][0] = data.level2_surface_reflectance_parameters.reflectance_mult_band[i].toDouble();
            m_reflectance_mult_add_arrays[i][1] = data.level2_surface_reflectance_parameters.reflectance_add_band[i].toDouble();
        }
        read_landsat_bands_data(file_names);
        m_geo.utmZone = data.projection_attributes.utm_zone.toDouble();
        m_geo.resX = data.projection_attributes.grid_cell_size_reflective.toDouble();
        m_geo.resY = -m_geo.resX;
        m_geo.ulX = data.projection_attributes.corner_ul_projection_x_product.toDouble();
        m_geo.ulY = data.projection_attributes.corner_ul_projection_y_product.toDouble();
        if(!data.isHeaderValid)return;
        isHeaderValid = true;
    }
    if(isHeaderValid == false){
        m_satelite_type = sad::UNKNOWN_SATELLITE;
        return;
    }


    //fill universal struct instead of specific (WORK IN PROGRESS)
    for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
        if(m_landsat9_missed_channels[sad::sorted_landsat_bands_order_by_wavelength[i]])continue;
        qDebug()<<"----check test---"<<sad::landsat_bands_gui_names[sad::sorted_landsat_bands_order_by_wavelength[i]];
    }



    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(landsat_gui_available_bands,
                                                            ui->verticalLayout_satellite_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1,2,3});
    connect(m_dynamic_checkboxes_widget,
            SIGNAL(choosed_bands_changed()),
            this,SLOT(change_bands_and_show_image()));


    change_bands_and_show_image();
    ui->statusbar->showMessage("");
    m_is_image_created = true;
    m_scene_cross_square_item->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);

}

void MainWindowSatelliteComparator::openCommonSentinelHeaderData(const QString& satellite_name)
{
    QString openSatMessage = QString("Открыть заголовочный файл %1").arg(satellite_name);
    QString headerName =  QFileDialog::getOpenFileName(this,openSatMessage,"",
                                                       "файлы(MTD_MSIL2A.xml)");


    ui->graphicsView_satellite_image->setIsSignal(false);
    clearLandsat9DataBands();
    clear_satellite_data();
    clear_all_layers();
    m_scene_cross_square_item->setVisible(false);

    QFile file(headerName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл Sentinel XML";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Ошибка разбора XML";
        file.close();
        return;
    }
    file.close();
    QFileInfo fi(headerName);
    m_root_path = fi.path();
    QString dataLoadingMessage = QString("Загрузка данных %1...").arg(satellite_name);
    ui->statusbar->showMessage(dataLoadingMessage);
    QApplication::processEvents();

    QStringList imageFiles;
    QDomNodeList imageNodes = doc.elementsByTagName("IMAGE_FILE");
    for (int i = 0; i < imageNodes.count(); ++i) {
        QDomNode node = imageNodes.at(i);
        imageFiles << node.toElement().text();
    }
    QStringList filteredFiles;

    for (const QString& file : qAsConst(imageFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            // Ищем точное вхождение ключа как часть имени файла
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                filteredFiles << file;
                break; // нашли — переходим к следующему файлу
            }
        }
    }
    QStringList finalFiles;
    QMap<QString, QString> bestResolutionForBand;
    const QStringList priorityOrder = { "R20m","R10m","R60m" };

    // Для каждого band ищем путь с наивысшим приоритетом по разрешению
    for (const QString& bandKey : sad::sentinel_bands_keys) {
        for (const QString& resolution : priorityOrder) {
            for (const QString& file : qAsConst(filteredFiles)) {
                if (file.contains(resolution) && file.contains("_" + bandKey + "_")) {
                    bestResolutionForBand[bandKey] = file;
                    break; // нашли лучший — переходим к следующему band
                }
            }
            if (bestResolutionForBand.contains(bandKey)) break; // если уже найден — не ищем в меньших разрешениях
        }
    }

    // Собираем финальный список
    finalFiles = bestResolutionForBand.values();
    qDebug()<<finalFiles;
    title_satellite_name->setText(satellite_name);

    for (const QString& file : qAsConst(finalFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                m_sentinel_metadata.sentinel_missed_channels[i] = false; // Канал найден — не пропущен
                m_sentinel_metadata.files[i] = file;
                break;
            }
        }
    }

    if(m_dynamic_checkboxes_widget)m_dynamic_checkboxes_widget->clear();
    QList<QString> availableBandNames;
    QString gui_channels[SENTINEL_BANDS_NUMBER];
    double central_waves[SENTINEL_BANDS_NUMBER];
    if(m_satelite_type == sad::SENTINEL_2A){
        copyQStringArray(sad::sentinel_2A_gui_band_names,gui_channels,SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2A_central_wave_lengths,sad::sentinel_2A_central_wave_lengths+SENTINEL_BANDS_NUMBER,central_waves);
    }else if(m_satelite_type == sad::SENTINEL_2B){
        copyQStringArray(sad::sentinel_2B_gui_band_names,gui_channels,SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2B_central_wave_lengths,sad::sentinel_2B_central_wave_lengths+SENTINEL_BANDS_NUMBER,central_waves);
    }

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        if (!m_sentinel_metadata.sentinel_missed_channels[i]) {
            availableBandNames << gui_channels[i];
            sad::BAND_DATA data;
            data.gui_name = gui_channels[i];
            data.central_wave_length = central_waves[i];
            data.file_name = m_sentinel_metadata.files[i];

            bool isResolutionMissed = true;
            for (const QString& resolution : priorityOrder) {
                if (data.file_name.contains(resolution)){
                    data.resolution_in_pixel_meters = resolution;
                    data.width = sad::sentinel_resolutions.value(resolution).first;
                    data.height = sad::sentinel_resolutions.value(resolution).second;
                    isResolutionMissed = false;
                    qDebug()<<"r, w, h: "<<data.resolution_in_pixel_meters<<data.width<<data.height;
                    break;
                };
            }
            if(isResolutionMissed){

                //TODO EXCEPTION
                //Мы обязательно должны знать разрешение
                // Выбросить исключение
                qDebug()<<"<--------------------- NO RESOLUTION EXCEPTION !!!------------------>";

            }
            m_sentinel_data.append(data);
        }
    }
    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(availableBandNames,
                                                            ui->verticalLayout_satellite_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1,2,3});

    connect(m_dynamic_checkboxes_widget,
            SIGNAL(choosed_bands_changed()),
            this,SLOT(change_bands_sentinel_and_show_image()));

    read_sentinel2_bands_data();
    change_bands_sentinel_and_show_image();

    ui->statusbar->showMessage("");
    m_is_image_created = true;
    m_scene_cross_square_item->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);

    if(finalFiles.empty()==false){
        QFileInfo finfo(m_root_path + "/" + finalFiles[0]+".jp2");
        QDir dir(finfo.absolutePath());
        dir.cdUp();
        dir.cdUp();
        const QString geo_file = dir.path()+"/MTD_TL.xml";
        fi.setFile(geo_file);
        auto xml_doc = fi.absoluteFilePath();
        qDebug()<<xml_doc<<"--->"<<fi.exists();
        m_geo.utmZone = extractUTMZoneFromXML(xml_doc);
        sentinel_geo = extractGeoPositions(xml_doc);
        m_geo.ulX = sentinel_geo["20"].ulX;
        m_geo.ulY = sentinel_geo["20"].ulY;
        m_geo.resX = 20;
        m_geo.resY = -20;
    }
}

void MainWindowSatelliteComparator::processBekasDataForComparing(const QVector<double>& x,
                                                                 const QVector<double>& y)
{
    //qDebug()<<"sat_comparator: "<<x.size()<<y.size();
    if(m_satelite_type == sad::UNKNOWN_SATELLITE)return;
    m_sat_comparator->initial_fill_data_to_show(x,y,waves_landsat9,m_landsat9_sample);
    if(m_satelite_type == sad::LANDSAT_9){
        m_sat_comparator->set_satellite_responses("landsat9");
    }else if(m_satelite_type == sad::LANDSAT_8){
        m_sat_comparator->set_satellite_responses("landsat8");
    }
    auto folded_device_spectr_for_landsat = m_sat_comparator->fold_spectr_to_satellite_responses();
    if(folded_device_spectr_for_landsat.empty()){
        m_is_bekas = false;
        return;
    }
    m_bekas_sample = folded_device_spectr_for_landsat;
    m_is_bekas = true;
}

void MainWindowSatelliteComparator::handleJsonRpcResult(const QJsonValue &result)
{
    qDebug()<<"call handleJsonRpcResult from MainWindowSatelliteComparator";
    qDebug()<<"result:"<< result.toString();
}

void MainWindowSatelliteComparator::processTestMatlabRequest(const QVariantMap &params)
{
    QString path = params["path"].toString();
    int     mode = params["mode"].toInt();
    qDebug()<<"matlab вызывал Test и передал параметры: "<<path<<" "<<mode;
}

void MainWindowSatelliteComparator::processpClassifiedBecasSpectraMatlabRequest(const QVariantMap &params)
{
    QString pathMatfile = params["matFilePath"].toString();
    MatFilesOperator reader;
    BecasDataFromMatlab dataReaded =  reader.readBecasDataFromMatlab(pathMatfile);
    if(dataReaded.isSomeErrors){
        qDebug()<<"при чтении файла произошли ошибки";
        return;
    }

    bekas_window->updateListWithClustNums(dataReaded.specNames,
                                          dataReaded.selectedClustIndxs,
                                          dataReaded.colorsOfEachSpectr);
}

void MainWindowSatelliteComparator::updateImage()
{
    if(m_image_item){
        double coef_saturation = ui->widget_image_saturation_light_corrector->getCoefSaturation();
        double coef_light = ui->widget_image_saturation_light_corrector->getCoefLight();
        if(coef_light == 1 && coef_saturation == 1) return;
        QImage imgNew = createModifiedImage(m_satellite_image, coef_saturation, coef_light);
        auto pixmap = QPixmap::fromImage(imgNew);
        m_scene->removeItem(m_image_item);
        delete m_image_item;
        m_image_item = new QGraphicsPixmapItem(pixmap);
        m_image_item->setZValue(0);
        m_scene->addItem(m_image_item);
        m_scene->update();
    }
}

QStringList MainWindowSatelliteComparator::getLandSat9BandsFromTxtFormat(const QString& path,
                                                                         QList<QString>& available_gui_bands)
{
    QStringList bands;
    QFile file(path);
    if(file.exists()==false)return bands;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)==false)return bands;
    QString temp;
    QStringList bands_lines;
    bool isGroupProductContentsStart = false;
    while(ts.readLineInto(&temp)){
        if(temp.contains("GROUP = PRODUCT_CONTENTS")){isGroupProductContentsStart = true;}
        if(temp.contains("END_GROUP = PRODUCT_CONTENTS")){break;}
        if(isGroupProductContentsStart==false)continue;
        if(temp.contains("FILE_NAME_BAND_")){
            bands_lines.append(temp);
            temp = temp.mid(temp.indexOf('"'),temp.lastIndexOf('"'));
            temp.replace('"',"");
            bands.append(temp);
        }
    }
    //qDebug()<<bands_lines;
    for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
        QString searchString = sad::landsat9_bands_keys[i];
        bool found = false;
        for (const QString &item : bands_lines)
        {
            if (item.contains(searchString, Qt::CaseSensitive)) {
                found = true;
                //qDebug()<<item<<"********** MATCHED ****************"<<searchString;
                break;
            }
        }
        if(found==false){
            qDebug()<<"missed band: "<<sad::landsat9_bands_keys[i];
            m_landsat9_missed_channels[i] = true;
        }else{
            m_landsat9_missed_channels[i] = false;
            available_gui_bands.append(sad::landsat_bands_gui_names[i]);
        }
    }
    return bands;
}

QString MainWindowSatelliteComparator::getLandSatSpaceCraftIDFromTxtFormat(const QString &path)
{
    QFile file(path);
    if(file.exists()==false)return "";
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)==false)return "";
    QString temp;
    QStringList bands_lines;
    bool isGroupImageAttributes = false;
    while(ts.readLineInto(&temp)){
        if(temp.contains("GROUP = IMAGE_ATTRIBUTES")){isGroupImageAttributes = true;}
        if(temp.contains("END_GROUP = IMAGE_ATTRIBUTES")){break;}
        if(isGroupImageAttributes==false)continue;
        if(temp.contains("SPACECRAFT_ID")){
            bands_lines.append(temp);
            temp = temp.mid(temp.indexOf('"'),temp.lastIndexOf('"'));
            temp.replace('"',"");
            return temp;
        }
    }
    return "";
}

void MainWindowSatelliteComparator::fillLandSat9ReflectanceMultAdd(const QString& path)
{
    QFile file(path);
    if(file.exists()==false) return;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)==false) return;
    QString temp;
    QVector<double> mult;
    QVector<double> add;
    bool isReflectanceGroup = false;
    while(ts.readLineInto(&temp)){
        temp = temp.trimmed();
        if(temp=="GROUP = LEVEL2_SURFACE_REFLECTANCE_PARAMETERS"){
            isReflectanceGroup = true;
            continue;
        }
        if(isReflectanceGroup==false)continue;
        if(temp=="END_GROUP = LEVEL2_SURFACE_REFLECTANCE_PARAMETERS"){
            break;
        }
        if(temp.contains("REFLECTANCE_MULT_BAND_")){
            temp = temp.mid(temp.indexOf("= "),temp.size()-1);
            temp.replace("= ","");
            mult.append(temp.toDouble());
        }else
            if(temp.contains("REFLECTANCE_ADD_BAND_")){
                temp = temp.mid(temp.indexOf("= "),temp.size()-1);
                temp.replace("= ","");
                add.append(temp.toDouble());
            }
    }

    if(mult.size()!=add.size()){
        qDebug()<<"SIZES ARE NOT THE SAME....";
        return;
    }
    if(mult.size()>LANDSAT_BANDS_NUMBER){
        qDebug()<<"SIZE TOO BIG...";
        return;
    }
    for(int i=0;i<mult.size();++i){
        m_reflectance_mult_add_arrays[i][0] = mult[i];
        m_reflectance_mult_add_arrays[i][1] = add[i];
    }


}

void MainWindowSatelliteComparator::fillLandSat9GeoData(const QString &path)
{
    QFile file(path);
    if(file.exists()==false) return;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)==false) return;
    QString temp;
    bool isProjectionGroup = false;
    while(ts.readLineInto(&temp)){
        if(temp.contains("GROUP = PROJECTION_ATTRIBUTES")){
            isProjectionGroup = true;
            continue;
        }
        if(isProjectionGroup==false)continue;
        if(temp.contains("END_GROUP = PROJECTION_ATTRIBUTES")){
            break;
        }
        if(temp.contains("CORNER_UL_PROJECTION_X_PRODUCT")){
            temp = temp.mid(temp.indexOf("= "),temp.size()-1);
            temp.replace("= ","");
            m_geo.ulX = temp.toDouble();
        }else
            if(temp.contains("CORNER_UL_PROJECTION_Y_PRODUCT")){
                temp = temp.mid(temp.indexOf("= "),temp.size()-1);
                temp.replace("= ","");
                m_geo.ulY = temp.toDouble();
            }else
                if(temp.contains("UTM_ZONE")){
                    temp = temp.mid(temp.indexOf("= "),temp.size()-1);
                    temp.replace("= ","");
                    m_geo.utmZone = temp.toDouble();
                }else
                    if(temp.contains("GRID_CELL_SIZE_REFLECTIVE")){
                        temp = temp.mid(temp.indexOf("= "),temp.size()-1);
                        temp.replace("= ","");
                        m_geo.resX = temp.toDouble();
                        m_geo.resY = - m_geo.resX;
                    }
    }
}

void MainWindowSatelliteComparator::clearLandsat9DataBands()
{
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        if(m_landsat9_missed_channels[i])continue;
        if(m_landsat9_data_bands[i]==nullptr)continue;
        delete[] m_landsat9_data_bands[i];
        m_landsat9_missed_channels[i]=true;
        m_landsat9_data_bands[i] = nullptr;
    }
}

uint16_t* MainWindowSatelliteComparator::readTiff(const QString& path,
                                                  int& xSize,
                                                  int& ySize)
{
    QString imgPath = path;
    QByteArray ba = imgPath.toUtf8();
    QFile file(imgPath);
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    bool isFileExists = file.exists();

    if(false==isFileExists){
        qDebug()<<"File exists: -->"<<path<<isFileExists;
        return nullptr;
    }
    const char *fileName = ba.constData();

    GDALDataset* poDataset = (GDALDataset*) GDALOpen( fileName, GA_ReadOnly );
    GDALRasterBand* poBand;
    poBand = poDataset->GetRasterBand(1);
    xSize = poBand->GetXSize();
    ySize = poBand->GetYSize();
    uint16_t* raster = new uint16[xSize*ySize];
    poBand->RasterIO(GF_Read, 0, 0, xSize, ySize, raster, xSize, ySize, GDT_UInt16, 0, 0);
    GDALClose(poDataset);
    return raster;

}

void MainWindowSatelliteComparator::read_landsat_bands_data(const QStringList& file_names)
{
    for(int i=0;i<file_names.size();++i){
        auto band_file_name = file_names[i];
        int xS = 0;
        int yS = 0;
        m_landsat9_data_bands[i] = readTiff(m_root_path+"/"+band_file_name,xS,yS);
        //qDebug()<<"x y -- sizes: "<<xS<<yS;
        m_landsat9_bands_image_sizes[i] = {xS,yS};
    }
}

QVector<double> MainWindowSatelliteComparator::getLandsat8Speya(const int x,
                                                                const int y)
{
    if(m_is_image_created==false)return {};
    int xSize= m_landsat9_bands_image_sizes->first;
    int ySize= m_landsat9_bands_image_sizes->second;
    if(x>xSize||y>ySize) return {};
    if(x<0||y<0) return {};
    QVector<double> speya;
    for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
        if(i==7||i==9||i==10)continue;// пропускаем каналы panchrom, и последние два LWR-100m
        uint16_t value = m_landsat9_data_bands[i][(y*xSize) + x];
        double speya_d = m_radiance_mult_add_arrays[i][0]*value+m_radiance_mult_add_arrays[i][1];
        speya.append(speya_d);
    }
    //qDebug()<<"speya: --> "<<speya;
    return speya;
}

inline QVector<double> MainWindowSatelliteComparator::getLandsat8Ksy(const int x,
                                                                     const int y)
{
    if(m_is_image_created==false)return {};
    int xSize= m_landsat9_bands_image_sizes->first;
    int ySize= m_landsat9_bands_image_sizes->second;
    if(x>xSize||y>ySize) return {};
    if(x<0||y<0) return {};
    QVector<double> ksy;
    for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
        if(i==7||i==8||i==9||i==10)continue;// пропускаем каналы panchrom, и последние два LWR-100m
        uint16_t value = m_landsat9_data_bands[i][(y*xSize) + x];
        double ksy_d = m_reflectance_mult_add_arrays[i][0]*value+m_reflectance_mult_add_arrays[i][1];
        ksy.append(ksy_d);
    }
    return ksy;
}

void MainWindowSatelliteComparator::paintSamplePoints(const QColor& color)
{

    int xSize = 0;
    int ySize = 0;
    QVector<double>sample;

    if(m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_8||m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_9){
        xSize = m_landsat9_bands_image_sizes->first;
        ySize = m_landsat9_bands_image_sizes->second;
        sample = m_landsat9_sample;
    }else if(m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2A||m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2B){
        if(m_sentinel_data.empty())return;//MESSAGE WARNING
        xSize = m_sentinel_data[0].width;
        ySize = m_sentinel_data[0].height;
        sample = m_sentinel_sample;
        if(sample.empty())return;
    };

    int total_pixels = xSize*ySize;


    if(m_is_bekas){
        sample = m_bekas_sample;
    }

    auto new_layer = new uchar[total_pixels*4];

    int midY = ySize / 2;
    int offset1 = 0;
    int offset2 = midY * xSize * 4;

    std::thread t1(&MainWindowSatelliteComparator::processLayer,this, new_layer, xSize, 0, midY, sample, color, offset1);
    std::thread t2(&MainWindowSatelliteComparator::processLayer,this, new_layer, xSize, midY, ySize, sample, color, offset2);
    t1.join();
    t2.join();
    auto cleanup = [](void* info) {
        delete[] static_cast<uchar*>(info);
    };
    auto img = QImage(new_layer,xSize,ySize,xSize*4,QImage::Format_RGBA8888,cleanup,new_layer);
    auto pixmap = QPixmap::fromImage(img);
    auto new_image_item = new QGraphicsPixmapItem(pixmap);
    new_image_item->setZValue(ui->graphicsView_satellite_image->getMaxZValue(m_scene));
    m_scene->addItem(new_image_item);
    ui->graphicsView_satellite_image->centerOn(m_scene_cross_square_item);


    const QString searchParams = m_comboBox_calculation_method->currentText() + ": "+QString::number(euclid_param_spinbox->value());
    auto stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd/hh:mm:ss");
    m_layers_search_result_items.insert(stamp,new_image_item);
    m_layer_gui_list->addItemToList(stamp,searchParams,color);
}

QString MainWindowSatelliteComparator::getGeoCoordinates(const int x,
                                                         const int y)
{

    // Строим геотрансформацию (предполагается, что изображение не имеет поворота)
    double geoTransform[6] = {
        m_geo.ulX,            // Верхний левый X (восточное направление)
        m_geo.resX,           // Разрешение по X
        0,                    // Поворот (обычно 0 для Landsat)
        m_geo.ulY,            // Верхний левый Y (северное направление)
        0,                    // Поворот
        m_geo.resY                 // Разрешение по Y (отрицательное, т.к. ось Y направлена вниз)
    };

    // Создаем проекцию UTM
    OGRSpatialReference utmSrs;
    utmSrs.SetProjCS("UTM");
    utmSrs.SetWellKnownGeogCS("WGS84"); // DATUM из MTL.json
    utmSrs.SetUTM(m_geo.utmZone, true);   // Северное - true или южное - false полушарие

    // Создаем целевую проекцию (WGS84)
    OGRSpatialReference wgs84Srs;
    wgs84Srs.SetWellKnownGeogCS("WGS84");

    // Создаем преобразователь координат
    OGRCoordinateTransformation* transformer =
            OGRCreateCoordinateTransformation(&utmSrs, &wgs84Srs);

    // Вычисляем координаты в проекции UTM
    double utmX = geoTransform[0] + x * geoTransform[1] + y * geoTransform[2];
    double utmY = geoTransform[3] + x * geoTransform[4] + y * geoTransform[5];

    // Преобразуем UTM -> WGS84 (широта/долгота)
    double lon = utmX;
    double lat = utmY;
    if (!transformer->Transform(1, &lon, &lat)) {
        return "";
    }

    //qDebug()<< "Географические координаты (WGS84):";
    //qDebug()<< "Долгота: " << lon;
    //qDebug()<< "Широта: " << lat;
    m_lattitude = lat;
    m_longitude = lon;
    QString lat_lon = QString("Широта: %1 Долгота: %2").arg(lat).arg(lon);
    OCTDestroyCoordinateTransformation(transformer);
    ui->statusbar->showMessage(lat_lon);
    return lat_lon;

}

inline double MainWindowSatelliteComparator::euclideanDistance(const QVector<double> &v1,
                                                               const QVector<double> &v2)
{
    if (v1.size() != v2.size()) {
        throw std::invalid_argument("Векторы должны быть одинаковой длины");
    }

    double sum = 0.0;
    for (int i = 0; i < v1.size(); ++i) {
        sum += qPow(v1[i] - v2[i], 2);
    }

    return qSqrt(sum);
}

inline double MainWindowSatelliteComparator::calculateSpectralAngle(const QVector<double> &S1,
                                                                    const QVector<double> &S2)
{
    if (S1.size() != S2.size()) {
        return -1;
    }

    double dotProduct = 0.0;
    double normS1 = 0.0;
    double normS2 = 0.0;

    for (int i = 0; i < S1.size(); ++i) {
        dotProduct += S1[i] * S2[i];
        normS1 += S1[i] * S1[i];
        normS2 += S2[i] * S2[i];
    }

    normS1 = qSqrt(normS1);
    normS2 = qSqrt(normS2);

    double cosineTheta = dotProduct / (normS1 * normS2);

    return qAcos(cosineTheta) * 180.0 / M_PI; // Возвращаем угол в градусах
}

void MainWindowSatelliteComparator::showGoogleMap()
{
    std::string command = "start ";
    command.append(maps_utility::makeGoogleUrl(m_lattitude,
                                               m_longitude));
    system(command.c_str());
}

void MainWindowSatelliteComparator::resetColorsToDefaultRGB()
{
    ui->widget_image_saturation_light_corrector->setDefaultValues();
    if(m_dynamic_checkboxes_widget){
        m_dynamic_checkboxes_widget->setRGBchannels();
        if(m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 || m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9){
            change_bands_and_show_image();
        }else if(m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A || m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B){
            change_bands_sentinel_and_show_image();
        }
    }
}


void MainWindowSatelliteComparator::change_bands_and_show_image()
{
    auto bands = m_dynamic_checkboxes_widget->get_choosed_bands();
    const int nXSize = m_landsat9_bands_image_sizes->first;
    const int nYSize = m_landsat9_bands_image_sizes->second;

    ProgressInformator progress_info(ui->graphicsView_satellite_image,
                                     satc::message_changing_bands);
    progress_info.show();
    QApplication::processEvents();

    int offset = 0;
    for (int y = 0; y < nYSize; ++y) {
        for (int x = 0; x < nXSize; ++x) {
            int B = 0;
            int G = 0;
            int R = 0;
            for(int j=0;j<bands.size();++j){

                int choosedColor = -1;
                if(bands[j].second==BLUE){
                    B = static_cast<int>(m_landsat9_data_bands[bands[j].first][y * nXSize + x] / 255.0)*1;
                    choosedColor = BLUE;
                }else if(bands[j].second==GREEN){
                    G = static_cast<int>(m_landsat9_data_bands[bands[j].first][y * nXSize + x] / 255.0)*1;
                    choosedColor = GREEN;
                }else if(bands[j].second==RED){
                    R = static_cast<int>(m_landsat9_data_bands[bands[j].first][y * nXSize + x] / 255.0)*1;
                    choosedColor = RED;
                }
                if(bands.size() == 1){
                    switch (choosedColor) {
                    case RED:
                        G=R;
                        B=R;
                        break;
                    case GREEN:
                        B=G;
                        R=G;
                        break;
                    case BLUE:
                        R=B;
                        G=B;
                        break;
                    default:
                        break;
                    }
                }

            }
            m_image_data[offset]=R;
            m_image_data[offset+1]=G;
            m_image_data[offset+2]=B;
            offset = offset + 3;
        }
    }
    if(m_image_item){
        qDebug()<<"Delete image item....";
        m_scene->removeItem(m_image_item);// удаление со сцены
        delete m_image_item;            // освобождение памяти
    }
    m_satellite_image = QImage(m_image_data,nXSize,nYSize,nXSize*3,QImage::Format_RGB888);
    auto pixmap = QPixmap::fromImage(m_satellite_image);
    m_image_item = new QGraphicsPixmapItem(pixmap);
    m_image_item->setCursor(Qt::CrossCursor);
    m_image_item->setZValue(Z_INDEX_BASE_IMAGE);
    m_scene->addItem(m_image_item);
    m_scene->setSceneRect(pixmap.rect());
    ui->graphicsView_satellite_image->centerOn(m_image_item);
    updateImage();
}

void MainWindowSatelliteComparator::change_bands_sentinel_and_show_image()
{
    // DUBLICATED CODE REFACTORING !!!!!!
    auto bands = m_dynamic_checkboxes_widget->get_choosed_bands();
    auto best_resolution = QPair<int,int>(5490,5490);//HARDCODED MAKE IT FLEXIBLE
    const int nXSize = best_resolution.first;
    const int nYSize = best_resolution.second;

    ProgressInformator progress_info(ui->graphicsView_satellite_image,
                                     satc::message_changing_bands);
    progress_info.show();
    QApplication::processEvents();

    int offset = 0;
    for (int y = 0; y < nYSize; ++y) {
        for (int x = 0; x < nXSize; ++x) {
            int B = 0;
            int G = 0;
            int R = 0;
            for(int j=0;j<bands.size();++j){
                int choosedColor = -1;
                if(bands[j].second==BLUE){
                    B = static_cast<int>(m_sentinel_data[bands[j].first].data[y * nXSize + x] / 255.0)*2;
                    choosedColor = BLUE;
                }else if(bands[j].second==GREEN){
                    G = static_cast<int>(m_sentinel_data[bands[j].first].data[y * nXSize + x] / 255.0)*2;
                    choosedColor = GREEN;
                }else if(bands[j].second==RED){
                    R = static_cast<int>(m_sentinel_data[bands[j].first].data[y * nXSize + x] / 255.0)*2;
                    choosedColor = RED;
                }
                if(bands.size() == 1){
                    switch (choosedColor) {
                    case RED:
                        G=R;
                        B=R;
                        break;
                    case GREEN:
                        B=G;
                        R=G;
                        break;
                    case BLUE:
                        R=B;
                        G=B;
                        break;
                    default:
                        break;
                    }
                }

            }
            m_image_data[offset]=R;
            m_image_data[offset+1]=G;
            m_image_data[offset+2]=B;
            offset = offset+3;
        }
    }
    if(m_image_item){
        m_scene->removeItem(m_image_item);
        delete m_image_item;
    }
    m_satellite_image = QImage(m_image_data,nXSize,nYSize,nXSize*3,QImage::Format_RGB888);
    auto pixmap = QPixmap::fromImage(m_satellite_image);
    m_image_item = new QGraphicsPixmapItem(pixmap);
    m_image_item->setCursor(Qt::CrossCursor);
    m_image_item->setZValue(Z_INDEX_BASE_IMAGE);
    m_scene->addItem(m_image_item);
    m_scene->setSceneRect(pixmap.rect());
    ui->graphicsView_satellite_image->centerOn(m_image_item);
    updateImage();
}

void MainWindowSatelliteComparator::show_layer(const QString& id)
{
    m_layers_search_result_items.value(id)->setVisible(true);
}

void MainWindowSatelliteComparator::hide_layer(const QString& id)
{
    m_layers_search_result_items.value(id)->setVisible(false);
}

void MainWindowSatelliteComparator::remove_scene_layer(const QString& id)
{
    auto image_item = m_layers_search_result_items.value(id);

    if(image_item) {
        m_scene->removeItem(image_item);
        delete image_item;
        m_layers_search_result_items.remove(id);
    }
}

void MainWindowSatelliteComparator::add_roi_to_gui_list(const QString &id)
{
    m_layer_roi_list->addItemToList(id,"Класс по умолчанию",QColor(Qt::yellow));
}

void MainWindowSatelliteComparator::show_roi_average(const QString &id)
{
    qDebug()<<"TEST ROI POLYGON INTERSECTION CONNECTION....";
    auto polItem = ui->graphicsView_satellite_image->getPolygonById(id);
    auto points = ui->graphicsView_satellite_image->getPointsInsidePolygon(polItem,m_image_item);
    qDebug()<<"POINTS SIZE: "<<points.size();
    for(int i=0;i<points.size();++i){
        qDebug()<<points[i].x()<<points[i].y();
    }
}

void MainWindowSatelliteComparator::processLayer(uchar* layer,
                                                 int xSize,
                                                 int yStart,
                                                 int yEnd,
                                                 const QVector<double> sample,
                                                 QColor color,
                                                 int offsetStart) {
    int offset = offsetStart;
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = 0; x < xSize; ++x) {
            QVector<double> ksy;
            if(m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_8||m_satelite_type==sad::SATELLITE_TYPE::LANDSAT_9){
                ksy = getLandsat8Ksy(x, y);
            }else if(m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2A||m_satelite_type==sad::SATELLITE_TYPE::SENTINEL_2B){
                auto w_k = getSentinelKsy(x, y);
                ksy = w_k.second;
            }
            if (m_is_bekas) {
                size_t elems_to_copy = std::min(static_cast<size_t>(ksy.size()), static_cast<size_t>(5));//TO DO DEFINE NUMBER OF CHANNELS
                std::vector<double> temp(ksy.begin(), ksy.begin() + elems_to_copy);
                ksy = QVector<double>::fromStdVector(temp);
            }

            double result = 999;
            if (m_comboBox_calculation_method->currentText() == satc::spectral_angle) {
                result = calculateSpectralAngle(ksy, sample);
            } else if (m_comboBox_calculation_method->currentText() == satc::euclid_metrika) {
                result = euclideanDistance(ksy, sample);
            }

            layer[offset]     = color.red();
            layer[offset + 1] = color.green();
            layer[offset + 2] = color.blue();
            layer[offset + 3] = result < euclid_param_spinbox->value() ? 255 : 0;

            offset += 4;
        }
    }
}

void MainWindowSatelliteComparator::initSentinelStructs()
{
    m_sentinel_metadata.isHeaderValid = false;
    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        m_sentinel_metadata.sentinel_missed_channels[i] = true; // Изначально считаем все каналы пропущенными
    }
}

void MainWindowSatelliteComparator::initLandsatStructs()
{
    std::fill(std::begin(m_landsat9_missed_channels),std::end(m_landsat9_missed_channels),true);
    m_landsat9_sample = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

void MainWindowSatelliteComparator::setUpPreviewPlot()
{
    m_preview_plot = new QCustomPlot;
    m_preview_plot->setFixedSize(400,200);
    m_preview_plot->legend->setVisible(true);
    QCPGraph *graph_satellite = m_preview_plot->addGraph();
    m_preview_plot->xAxis->setRange(400,2400);
    m_preview_plot->yAxis->setRange(-0.2,1.2);
    m_preview_plot->xAxis->setLabel("Длина волны, nm");
    m_preview_plot->yAxis->setLabel("КСЯ");
    // Создаем заголовок
    title_satellite_name = new QCPTextElement(m_preview_plot,
                                              "",
                                              QFont("Arial", 10,
                                                    QFont::Bold));
    m_preview_plot->plotLayout()->insertRow(0);
    m_preview_plot->plotLayout()->addElement(0, 0, title_satellite_name);
    graph_satellite->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    QCPGraph *graph_device = m_preview_plot->addGraph();
    graph_device->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 5));
    m_preview_plot->graph(0)->setName("Курсорный");
    m_preview_plot->graph(1)->setName("Образец для поиска");
    m_preview_plot->graph(0)->setPen(QPen(Qt::blue));
    m_preview_plot->graph(1)->setPen(QPen(Qt::red));
}

void MainWindowSatelliteComparator::setUpToolWidget()
{
    QWidget* widget_tools = new QWidget(ui->graphicsView_satellite_image);
    m_comboBox_calculation_method = new QComboBox;
    m_comboBox_calculation_method->addItems({satc::euclid_metrika,satc::spectral_angle});

    m_layer_gui_list = new LayerList;
    connect(m_layer_gui_list,SIGNAL(showItem(const QString)),SLOT(show_layer(const QString)));
    connect(m_layer_gui_list,SIGNAL(hideItem(const QString)),SLOT(hide_layer(const QString)));
    connect(m_layer_gui_list,SIGNAL(removeItem(const QString)),SLOT(remove_scene_layer(const QString)));

    m_layer_roi_list = new LayerRoiList;
    ui->verticalLayout_roi->addWidget(m_layer_roi_list);
    connect(m_layer_roi_list,SIGNAL(showItem(const QString)),ui->graphicsView_satellite_image,SLOT(show_roi_layer(const QString)));
    connect(m_layer_roi_list,SIGNAL(hideItem(const QString)),ui->graphicsView_satellite_image,SLOT(hide_roi_layer(const QString)));
    connect(m_layer_roi_list,SIGNAL(removeItem(const QString)),ui->graphicsView_satellite_image,SLOT(remove_roi_scene_layer(const QString)));
    connect(m_layer_roi_list,SIGNAL(roi_color_changed(const QString, const QColor)),
            ui->graphicsView_satellite_image,SLOT(changeRoiColor(const QString, const QColor)));
    connect(m_layer_roi_list,SIGNAL(roi_item_selected(const QString)),
            ui->graphicsView_satellite_image,SLOT(setRoiSelectEffect(const QString)));
    connect(m_layer_roi_list,SIGNAL(roiPolygonAverage(const QString)),
            this,SLOT(show_roi_average(const QString)));


    QHBoxLayout* tool_root_layout = new QHBoxLayout;
    QVBoxLayout* toolLayOut = new QVBoxLayout;
    tool_root_layout->addLayout(toolLayOut);
    const QSize tool_element_size(30,30);


    QPushButton *pushbutton_centerOn = new QPushButton;
    pushbutton_centerOn->setText("●");
    pushbutton_centerOn->setFixedSize(tool_element_size);

    QPushButton *zoomInButton = new QPushButton;
    zoomInButton->setText("+");
    zoomInButton->setFixedSize(tool_element_size);
    QPushButton *zoomOutButton = new QPushButton;
    zoomOutButton->setText("-");
    zoomOutButton->setFixedSize(tool_element_size);
    QPushButton *googleMap = new QPushButton;
    googleMap->setText("GM");
    googleMap->setFixedSize(tool_element_size);

    QPushButton *resetToRGB = new QPushButton;
    resetToRGB->setText("RC");
    resetToRGB->setFixedSize(tool_element_size);

    QVBoxLayout *euclid_layout = new QVBoxLayout;
    QSpacerItem *spacer1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QSpacerItem *spacer2 = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    QPushButton *pushbutton_paint_samples = new QPushButton;
    pushbutton_paint_samples->setText("Запуск поиска");

    euclid_param_spinbox = new QDoubleSpinBox;
    euclid_layout->addWidget(m_comboBox_calculation_method);
    euclid_layout->addWidget(euclid_param_spinbox);
    euclid_layout->addItem(spacer1);
    euclid_layout->addWidget(pushbutton_paint_samples);
    euclid_layout->addItem(spacer2);
    euclid_param_spinbox->setMinimum(0.001);
    euclid_param_spinbox->setMaximum(100);
    euclid_param_spinbox->setSingleStep(0.001);
    euclid_param_spinbox->setValue(0.2);

    toolLayOut->addWidget(pushbutton_centerOn);
    toolLayOut->addWidget(zoomInButton);
    toolLayOut->addWidget(zoomOutButton);
    toolLayOut->addWidget(googleMap);
    toolLayOut->addWidget(resetToRGB);

    tool_root_layout->addWidget(m_preview_plot);
    widget_tools->setLayout(tool_root_layout);
    tool_root_layout->addLayout(euclid_layout);
    //tool_root_layout->addWidget(m_layer_gui_list);
    ui->verticalLayout_search_layers->addWidget(m_layer_gui_list);
    widget_tools->show();

    connect(pushbutton_centerOn,SIGNAL(clicked()),this,SLOT(centerSceneOnCrossSquare()));
    connect(zoomInButton, SIGNAL(clicked()),ui->graphicsView_satellite_image,SLOT(zoomIn()));
    connect(zoomOutButton, SIGNAL(clicked()),ui->graphicsView_satellite_image,SLOT(zoomOut()));
    connect(googleMap,SIGNAL(clicked()),this,SLOT(showGoogleMap()));
    connect(resetToRGB,SIGNAL(clicked()),this,SLOT(resetColorsToDefaultRGB()));
    connect(pushbutton_paint_samples,SIGNAL(clicked()),this,SLOT(findAreasUsingSelectedMetric()));

}

void MainWindowSatelliteComparator::makeConnectsForMenuActions()
{
    connect(ui->actionBekas,SIGNAL(triggered()),this,SLOT(openBekasSpectraData()));
    connect(ui->actionOpenLandsat9Header,SIGNAL(triggered()),this,SLOT(openLandsat9HeaderData()));
    connect(ui->actionOpenLandsat8Header,SIGNAL(triggered()),this,SLOT(openLandsat8HeaderData()));
    connect(ui->actionSentinel_2A,SIGNAL(triggered()),this,SLOT(openSentinel2AHeaderData()));
    connect(ui->actionSentinel_2B,SIGNAL(triggered()),this,SLOT(openSentinel2BHeaderData()));
    connect(ui->action_LoadTimeRow,SIGNAL(triggered()),this,SLOT(openTimeRowData()));
}

void MainWindowSatelliteComparator::addBaseItemsToScene()
{
    m_scene_text_item_metric_value->setDefaultTextColor(Qt::black);
    m_scene_text_item_metric_value->setFont(QFont("Arial", 12));
    m_scene_text_item_metric_value->setZValue(Z_INDEX_CROSS_SQUARE_CURSOR_TEXT);
    m_scene->addItem(m_scene_text_item_metric_value);
    m_scene_cross_square_item->setPos(0,0);
    m_scene_cross_square_item->setVisible(false);
    m_scene_cross_square_item->setZValue(Z_INDEX_CROSS_SQUARE_CURSOR);
    m_scene->addItem(m_scene_cross_square_item);
}

void MainWindowSatelliteComparator::read_sentinel2_bands_data()
{

    for (int i = 0; i < m_sentinel_data.size(); ++i) {
        const QString& band_file_name = m_sentinel_data[i].file_name;
        int xS = 0;
        int yS = 0;

        m_sentinel_data[i].data  = readTiff(m_root_path + "/" + band_file_name+".jp2",xS,yS);

        if(m_sentinel_data[i].resolution_in_pixel_meters=="R10m"){
            qDebug()<<"RESOLUTION 10 TO 20";
            int outX = xS/2;
            int outY = yS/2;
            // Выделяем буфер вручную
            uint16_t* buffer = new uint16_t[(sizeof(uint16_t) * outX * outY)];
            downsample_uint16(m_sentinel_data[i].data,buffer,xS,yS);
            delete []m_sentinel_data[i].data;
            m_sentinel_data[i].data = buffer;

        }
        qDebug() << "Sentinel band" << i << "size:" << xS << "x" << yS;
        if(m_sentinel_data[i].width != xS)qDebug()<<"WRONG X";
        if(m_sentinel_data[i].height != yS)qDebug()<<"WRONG Y";

    }


}

void MainWindowSatelliteComparator::gdal_start_driver()
{
    QString dataPath = QApplication::applicationDirPath() + "/data";
    CPLSetConfigOption("GDAL_DATA", dataPath.toUtf8().constData());
    GDALAllRegister();
    CPLSetConfigOption("GDAL_NUM_THREADS", "ALL_CPUS");
    CPLSetConfigOption("OPENJPEG_NUM_THREADS", "AUTO");
    CPLSetConfigOption("GDAL_CACHEMAX", "512");
    CPLSetConfigOption("CPL_VSIL_USE_TEMP_FILE", "NO");
    CPLSetConfigOption("GDAL_JP2KAK_USE", "YES");
}

void MainWindowSatelliteComparator::gdal_close_driver()
{
    GDALDestroyDriverManager();
}

QPair<QVector<double>, QVector<double> > MainWindowSatelliteComparator::getSentinelKsy(const int x,
                                                                                       const int y)
{
    if(m_is_image_created==false)return {};
    if(m_sentinel_data.empty())return {};
    int xSize= m_sentinel_data[0].width;
    int ySize= m_sentinel_data[0].width;
    if(x>xSize||y>ySize) return {};
    if(x<0||y<0) return {};
    QVector<double> ksy;
    QVector<double> waves;
    for(int i=0;i<m_sentinel_data.size();++i){
        uint16_t value = m_sentinel_data[i].data[(y*xSize) + x];
        //double ksy_d = m_reflectance_mult_add_arrays[i][0]*value+m_reflectance_mult_add_arrays[i][1];
        ksy.append(value/10000.0);
        waves.append(m_sentinel_data[i].central_wave_length);
    }
    return {waves,ksy};
}

void MainWindowSatelliteComparator::clear_satellite_data()
{
    if(m_sentinel_data.empty())return;
    for(int i=0;i<m_sentinel_data.size();++i){
        auto data = m_sentinel_data[i].data;
        if(data)delete[]data;
    }
    m_sentinel_data.clear();
}

void MainWindowSatelliteComparator::clear_all_layers()
{
    if(m_layers_search_result_items.empty())return;
    for (auto it = m_layers_search_result_items.constBegin(); it != m_layers_search_result_items.constEnd(); ++it) {
        QString key = it.key();
        remove_scene_layer(key);
    }
    m_layer_gui_list->clear();
    m_layers_search_result_items.clear();
}

QHash<QString, MainWindowSatelliteComparator::geoTransform> MainWindowSatelliteComparator::extractGeoPositions
(const QString &xmlFilePath)
{
    QHash<QString, geoTransform> positions;

    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << xmlFilePath;
        return positions;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Ошибка парсинга XML";
        file.close();
        return positions;
    }
    file.close();

    QDomNodeList geoNodes = doc.elementsByTagName("Geoposition");
    for (int i = 0; i < geoNodes.count(); ++i) {
        QDomElement geoElem = geoNodes.at(i).toElement();
        QString resolution = geoElem.attribute("resolution");

        geoTransform pos;
        pos.ulX = geoElem.firstChildElement("ULX").text().toDouble();
        pos.ulY = geoElem.firstChildElement("ULY").text().toDouble();
        positions.insert(resolution, pos);
    }

    return positions;
}

int MainWindowSatelliteComparator::extractUTMZoneFromXML(const QString &xmlFilePath)
{
    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << xmlFilePath;
        return -1;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Ошибка парсинга XML";
        file.close();
        return -1;
    }
    file.close();

    QDomNodeList nodes = doc.elementsByTagName("HORIZONTAL_CS_NAME");
    if (nodes.isEmpty()) {
        qWarning() << "Тег <HORIZONTAL_CS_NAME> не найден";
        return -1;
    }

    QString csName = nodes.at(0).toElement().text();
    QRegularExpression re("zone\\s*(\\d+)");
    QRegularExpressionMatch match = re.match(csName);

    if (match.hasMatch()) {
        return match.captured(1).toInt();  // Возвращает номер зоны
    }

    qWarning() << "UTM зона не найдена в строке:" << csName;
    return -1;
}

void MainWindowSatelliteComparator::getKSY(const QPointF &pos,
                                           QVector<double> &waves,
                                           QVector<double> &ksy)
{
    Q_UNUSED(pos)
    Q_UNUSED(waves)
    Q_UNUSED(ksy)
}

QImage MainWindowSatelliteComparator::createModifiedImage(const QImage &img,
                                                          double coefSat,
                                                          double coefLight)
{
    QImage adjusted = img.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy();
    const int pixelCount = adjusted.width() * adjusted.height();
    QRgb* pixels = reinterpret_cast<QRgb*>(adjusted.bits());

    const int threadCount = std::thread::hardware_concurrency();
    const int chunkSize = pixelCount / threadCount;

    std::vector<std::future<void>> futures;
    for (int i = 0; i < threadCount; ++i) {
        int start = i * chunkSize;
        int end = (i == threadCount - 1) ? pixelCount : start + chunkSize;
        futures.push_back(std::async(std::launch::async, [=] {
            for (int j = start; j < end; ++j) {
                QColor color(pixels[j]);
                qreal h, s, l;
                color.getHslF(&h, &s, &l);
                s = qBound(0.0, s * coefSat, 1.0);
                l = qBound(0.0, l * coefLight, 1.0);
                color.setHslF(h, s, l);
                pixels[j] = color.rgba();
            }
        }));
    }

    for (auto& f : futures) f.wait();
    return adjusted;
}
void MainWindowSatelliteComparator::initUdpRpcConnection()
{
    m_rpc = new UdpJsonRpc(LOCAL_PORT,
                           QHostAddress::LocalHost, MATLAB_PORT, this);

    // Регистрируем методы, которые может вызывать Matlab app.

    m_rpc->registerMethod("cppTestFunc",
                          [this](const QJsonValue &params) -> QJsonValue {
        qDebug()<<"cppTestFunc";
        QVariantMap map = params.toVariant().toMap();
        processTestMatlabRequest(map);
        return NULL;
    });

    m_rpc->registerMethod("processClassifiedBecasSpectra",
                          [this](const QJsonValue &params) -> QJsonValue {
        qDebug()<<"processClassifiedBecasSpectra";
        QVariantMap map = params.toVariant().toMap();
        processpClassifiedBecasSpectraMatlabRequest(map);
        return NULL;
    });


    // Подключаем сигнал получения результата к слоту обработки
    connect(m_rpc,
            &UdpJsonRpc::resultReady,
            this,
            &MainWindowSatelliteComparator::handleJsonRpcResult);
}
