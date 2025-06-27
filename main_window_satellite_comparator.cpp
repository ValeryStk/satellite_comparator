#include "main_window_satellite_comparator.h"
#include "ui_main_window_satellite_comparator.h"
#include <algorithm>
#include <QFile>
#include <QFileDialog>
#include <QString>
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
#include "string"


QCPTextElement *title_satellite_name;
QVector<double> waves_landsat9 = {443,482,562,655,865,1610,2200};
QVector<double> waves_landsat9_5 = {443,482,562,655,865};

QVector<double> sample;
QVector<double> waves;
QVector<double> trimmed_satellite_data;



MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , scene(new QGraphicsScene)
    , m_sat_comparator(new SatteliteComparator)
    , m_is_bekas(false)
    , cross_square(new CrossSquare(100))


{
    ui->setupUi(this);
    setWindowTitle(satc::app_name);
    bekas_window = nullptr;
    std::fill(std::begin(m_landsat9_missed_channels),std::end(m_landsat9_missed_channels),true);

    connect(ui->actionBekas,&QAction::triggered,[this](){
        bekas_window = new UasvViewWindow;
        bekas_window->setWindowTitle(satc::app_name);
        bekas_window->setAttribute(Qt::WA_DeleteOnClose);
        connect(bekas_window,SIGNAL(sendSampleForSatelliteComparator(QVector<double>, QVector<double>)),
                this,SLOT(processBekasDataForComparing(QVector<double>,QVector<double>)));
        bekas_window->show();
    });


    connect(ui->actionOpenLandsat9Header,&QAction::triggered,[this](){openLandsat9HeaderData();});
    connect(ui->actionOpenLandsat8Header,&QAction::triggered,[this](){openLandsat8HeaderData();});

    qgti = new QGraphicsTextItem;
    qgti->setDefaultTextColor(Qt::black);
    qgti->setFont(QFont("Arial", 12));
    qgti->setZValue(1001);
    scene->addItem(qgti);
    m_dynamic_checkboxes_widget = nullptr;

    calculation_method = new QComboBox;
    calculation_method->addItems({satc::euclid_metrika,satc::spectral_angle});
    cross_square->setPos(0,0);
    cross_square->setVisible(false);
    cross_square->setZValue(1000);
    scene->addItem(cross_square);
    m_landsat9_sample = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    preview = new QCustomPlot;
    preview->legend->setVisible(true);
    QCPGraph *graph_satellite = preview->addGraph();
    preview->xAxis->setRange(400,2400);
    preview->yAxis->setRange(-0.2,1.2);
    preview->xAxis->setLabel("Длина волны, nm");
    preview->yAxis->setLabel("КСЯ");
    // Создаем заголовок
    title_satellite_name = new QCPTextElement(preview,
                                              "",
                                              QFont("Arial", 10,
                                                    QFont::Bold));
    preview->plotLayout()->insertRow(0);
    preview->plotLayout()->addElement(0, 0, title_satellite_name);
    graph_satellite->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    QCPGraph *graph_device = preview->addGraph();
    graph_device->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 5));

    preview->graph(0)->setName("Курсорный");
    preview->graph(1)->setName("Образец для поиска");

    preview->graph(0)->setPen(QPen(Qt::blue));
    preview->graph(1)->setPen(QPen(Qt::red));

    connect(ui->graphicsView_satellite_image,&SatelliteGraphicsView::pointChanged,this, [this](QPointF pos){
        QVector<double> data = getLandsat8Ksy(pos.x(),pos.y());
        if(data.empty()){
            //qDebug()<<"DATA EMPTY!!!!";
            return;
        }
        if(data.size()!=(int)LANDSAT_9_BANDS_NUMBER-4){
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

        preview->graph(0)->data().clear();
        preview->graph(1)->data().clear();
        preview->graph(0)->setData(waves, trimmed_satellite_data);
        preview->graph(1)->setData(waves, sample);

        double result = 999;
        if(calculation_method->currentText()==satc::spectral_angle){
            result = calculateSpectralAngle(trimmed_satellite_data,sample);
        }else if(calculation_method->currentText()==satc::euclid_metrika){
            result = euclideanDistance(trimmed_satellite_data,sample);
        }
        qgti->setPos(pos.x(),pos.y()+5);
        qgti->setPlainText(QString::number(result));
        preview->rescaleAxes(true);
        preview->replot();
    });


    connect(ui->graphicsView_satellite_image,&SatelliteGraphicsView::sampleChanged,[this](QPointF pos){
        m_is_bekas = false;
        auto data = getLandsat8Ksy(pos.x(),pos.y());
        cross_square->setPos(pos);
        cross_square->update();
        if(data.empty())return;
        if(data.size()!=(int)LANDSAT_9_BANDS_NUMBER-4){
            qDebug()<<"ERROR SIZE:"<<data.size();
            return;
        }

        m_landsat9_sample = data;
        preview->graph(0)->data().clear();
        preview->graph(1)->data().clear();
        preview->graph(0)->setData(waves_landsat9, data);
        preview->graph(1)->setData(waves_landsat9, m_landsat9_sample);
        preview->rescaleAxes(true);
        preview->replot();
        getGeoCoordinates(pos.x(),pos.y());
    });



    ui->graphicsView_satellite_image->setMouseTracking(true);
    ui->graphicsView_satellite_image->setScene(scene);
    ui->graphicsView_satellite_image->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView_satellite_image->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_is_image_created = false;



    QWidget* widget_tools = new QWidget(ui->graphicsView_satellite_image);
    QHBoxLayout* tool_root_layout = new QHBoxLayout;
    QVBoxLayout* toolLayOut = new QVBoxLayout;
    tool_root_layout->addLayout(toolLayOut);
    const QSize tool_element_size(30,30);

    preview->setFixedSize(400,200);
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
    euclid_layout->addWidget(calculation_method);
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

    tool_root_layout->addWidget(preview);
    widget_tools->setLayout(tool_root_layout);
    tool_root_layout->addLayout(euclid_layout);
    widget_tools->show();
    QGraphicsProxyWidget* proxy = scene->addWidget(widget_tools);
    proxy->setPos(0, 50);
    proxy->setGeometry(QRect(0,0,600,250));
    QObject::connect(zoomInButton, &QPushButton::clicked, [this]() {
        ui->graphicsView_satellite_image->scale(1.2, 1.2); // Увеличение масштаба
    });
    QObject::connect(zoomOutButton, &QPushButton::clicked, [this]() {
        ui->graphicsView_satellite_image->scale(0.8, 0.8); // Уменьшение масштаба
    });

    connect(pushbutton_centerOn,&QPushButton::clicked,[this](){ // Центрирование
        ui->graphicsView_satellite_image->centerOn(cross_square);
        ui->graphicsView_satellite_image->setTransform(QTransform()); //Дефолтный масштаба

    });

    connect(pushbutton_paint_samples,&QPushButton::clicked,[this](){
        QColor color = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
        QString message = QString("Пожалуйста подождите,\nпроисходит поиск областей\n(%1)...").arg(calculation_method->currentText());
        ProgressInformator progress_info(ui->graphicsView_satellite_image,
                                         message);
        progress_info.show();
        QApplication::processEvents();
        paintSamplePoints(color);
        progress_info.close();

    });

    connect(googleMap,&QPushButton::clicked,[this](){
        showGoogleMap();
    });

    connect(resetToRGB,&QPushButton::clicked,[this](){
        if(m_dynamic_checkboxes_widget){
            m_dynamic_checkboxes_widget->setRGBchannels();
            change_bands_and_show_image();
        }
    });

}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
}

void MainWindowSatelliteComparator::openLandsat9HeaderData()
{
    openCommonLandsatHeaderData(satc::satellite_name_landsat_9);
}

void MainWindowSatelliteComparator::openLandsat8HeaderData()
{
    openCommonLandsatHeaderData(satc::satellite_name_landsat_8);
}

void MainWindowSatelliteComparator::openCommonLandsatHeaderData(const QString& satellite_name)
{
    QString openSatMessage = QString("Открыть заголовочный файл %1").arg(satellite_name);
    QString headerName =  QFileDialog::getOpenFileName(this,openSatMessage,"",
                                                       "JSON файлы(*_MTL.json *_MTL.txt *_MTL.xml)");
    ui->graphicsView_satellite_image->setIsSignal(false);
    clearLandsat9DataBands();
    cross_square->setVisible(false);
    m_satelite_type = sad::UKNOWN_SATELLITE;
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
    QList<QString> landsat9_gui_available_bands;

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

            for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
                if(check_bands.value(sad::landsat9_bands_keys[i]).isUndefined()){
                    qDebug()<<"missed band: "<<sad::landsat9_bands_keys[i];
                    m_landsat9_missed_channels[i] = true;
                    continue;
                }
                m_landsat9_missed_channels[i] = false;
                landsat9_gui_available_bands.append(sad::landsat9_bands_gui_names[i]);
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
                                                        landsat9_gui_available_bands);
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
        for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
            if(data.landsat9_missed_channels[i])continue;
            file_names.append(data.product_contents.file_name_bands[i]);
            landsat9_gui_available_bands.append(sad::landsat9_bands_gui_names[i]);
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
    if(isHeaderValid == false)return;

    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(landsat9_gui_available_bands,
                                                            ui->verticalLayout_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1,2,3});
    connect(m_dynamic_checkboxes_widget,
            SIGNAL(choosed_bands_changed()),
            this,SLOT(change_bands_and_show_image()));


    change_bands_and_show_image();
    ui->statusbar->showMessage("");
    m_is_image_created = true;
    cross_square->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);

}

void MainWindowSatelliteComparator::processBekasDataForComparing(const QVector<double>& x,
                                                                 const QVector<double>& y)
{
    //qDebug()<<"sat_comparator: "<<x.size()<<y.size();
    m_sat_comparator->initial_fill_data_to_show(x,y,waves_landsat9,m_landsat9_sample);
    m_sat_comparator->set_satellite_responses("landsat9");
    auto folded_device_spectr_for_landsat9 = m_sat_comparator->fold_spectr_to_satellite_responses();
    m_is_bekas = true;
    m_bekas_sample = folded_device_spectr_for_landsat9;
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
    for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
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
            available_gui_bands.append(sad::landsat9_bands_gui_names[i]);
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
            QString band_name = temp;
            temp = temp.mid(temp.indexOf("= "),temp.size()-1);
            temp.replace("= ","");
            mult.append(temp.toDouble());
        }else
            if(temp.contains("REFLECTANCE_ADD_BAND_")){
                QString band_name = temp;
                temp = temp.mid(temp.indexOf("= "),temp.size()-1);
                temp.replace("= ","");
                add.append(temp.toDouble());
            }
    }

    if(mult.size()!=add.size()){
        qDebug()<<"SIZES ARE NOT THE SAME....";
        return;
    }
    if(mult.size()>LANDSAT_9_BANDS_NUMBER){
        qDebug()<<"SIZE TOO BIG...";
        return;
    }
    for(int i=0;i<mult.size();++i){
        m_reflectance_mult_add_arrays[i][0] = mult[i];
        m_reflectance_mult_add_arrays[i][1] = add[i];
        //qDebug()<<"*******************--->"<<mult[i]<<add[i];
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
    for (int i = 0; i < LANDSAT_9_BANDS_NUMBER; ++i) {
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
    GDALAllRegister();
    GDALDataset* poDataset = (GDALDataset*) GDALOpen( fileName, GA_ReadOnly );
    GDALRasterBand* poBand;
    poBand = poDataset->GetRasterBand(1);
    xSize = poBand->GetXSize();
    ySize = poBand->GetYSize();
    uint16_t* raster = new uint16[xSize*ySize];
    poBand->RasterIO(GF_Read, 0, 0, xSize, ySize, raster, xSize, ySize, GDT_UInt16, 0, 0);
    GDALClose(poDataset);
    GDALDestroyDriverManager();
    return raster;

}

void MainWindowSatelliteComparator::read_landsat_bands_data(const QStringList& file_names)
{
    for(int i=0;i<file_names.size();++i){
        auto band_file_name = file_names[i];
        int xS;
        int yS;
        m_landsat9_data_bands[i] = readTiff(m_root_path+"/"+band_file_name,xS,yS);
        qDebug()<<"x y -- sizes: "<<xS<<yS;
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
    for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
        if(i==7||i==9||i==10)continue;// пропускаем каналы panchrom, и последние два LWR-100m
        uint16_t value = m_landsat9_data_bands[i][(y*xSize) + x];
        double speya_d = m_radiance_mult_add_arrays[i][0]*value+m_radiance_mult_add_arrays[i][1];
        speya.append(speya_d);
    }
    //qDebug()<<"speya: --> "<<speya;
    return speya;
}

QVector<double> MainWindowSatelliteComparator::getLandsat8Ksy(const int x,
                                                              const int y)
{
    if(m_is_image_created==false)return {};
    int xSize= m_landsat9_bands_image_sizes->first;
    int ySize= m_landsat9_bands_image_sizes->second;
    if(x>xSize||y>ySize) return {};
    if(x<0||y<0) return {};
    QVector<double> ksy;
    for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
        if(i==7||i==8||i==9||i==10)continue;// пропускаем каналы panchrom, и последние два LWR-100m
        uint16_t value = m_landsat9_data_bands[i][(y*xSize) + x];
        double ksy_d = m_reflectance_mult_add_arrays[i][0]*value+m_reflectance_mult_add_arrays[i][1];
        ksy.append(ksy_d);
    }

    int stop = 7;;
    return ksy;
}

void MainWindowSatelliteComparator::paintSamplePoints(const QColor& color)
{
    int xSize= m_landsat9_bands_image_sizes->first;
    int ySize= m_landsat9_bands_image_sizes->second;
    QVector<double>sample;
    if(m_is_bekas){
        sample = m_bekas_sample;
    }else{
        sample = m_landsat9_sample;
    }


    for(int i=0;i<xSize;++i){
        for(int j=0;j<ySize;++j){
            auto ksy = getLandsat8Ksy(i,j);

            if(m_is_bekas){// TODO FLEXIBLE NUMBER OF CHANNELS OPTIMIZATION
                size_t elems_to_copy = std::min(ksy.toStdVector().size(), static_cast<size_t>(5));
                ksy = QVector<double>(ksy.begin(), ksy.begin() + elems_to_copy);
            }
            double result = 999;
            if(calculation_method->currentText()==satc::spectral_angle){
                result = calculateSpectralAngle(ksy,sample);
            }else if(calculation_method->currentText()==satc::euclid_metrika){
                result = euclideanDistance(ksy,sample);
            }
            if(result<euclid_param_spinbox->value()){
                m_satellite_image.setPixelColor(QPoint(i,j),color);
            };
        }
    };
    scene->removeItem(m_image_item);
    auto pixmap = QPixmap::fromImage(m_satellite_image);
    m_image_item = new QGraphicsPixmapItem(pixmap);
    m_image_item->setCursor(Qt::CrossCursor);
    scene->addItem(m_image_item);
    scene->setSceneRect(pixmap.rect());
    cross_square->setZValue(1000);
    cross_square->update();
    ui->graphicsView_satellite_image->centerOn(cross_square);
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

double MainWindowSatelliteComparator::euclideanDistance(const QVector<double> &v1,
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

double MainWindowSatelliteComparator::calculateSpectralAngle(const QVector<double> &S1,
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


void MainWindowSatelliteComparator::change_bands_and_show_image()
{
    auto bands = m_dynamic_checkboxes_widget->get_choosed_bands();
    const int nXSize = m_landsat9_bands_image_sizes->first;
    const int nYSize = m_landsat9_bands_image_sizes->second;
    qDebug()<<"x -- y: "<<nXSize<<nYSize<<"is_image_ready: "<<m_is_image_created;
    //if(m_is_image_created==false){
    m_satellite_image = QImage(nXSize, nYSize, QImage::QImage::Format_RGB888);
    //}else{
    ProgressInformator progress_info(ui->graphicsView_satellite_image,
                                     satc::message_changing_bands);
    progress_info.show();
    //QApplication::processEvents();
    //}
    for (int y = 0; y < nYSize; ++y) {
        for (int x = 0; x < nXSize; ++x) {
            int B = 0;
            int G = 0;
            int R = 0;
            for(int j=0;j<bands.size();++j){
                // qDebug()<<"j band --> "<<bands[j].first;
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
                QRgb rgb = 0;
                if(bands.size() == 1){
                    switch (choosedColor) {
                    case RED:
                        rgb = qRgb(R,R,R);
                        break;
                    case GREEN:
                        rgb = qRgb(G,G,G);
                        break;
                    case BLUE:
                        rgb = qRgb(B,B,B);
                        break;
                    default:
                        break;
                    }
                }else{
                    rgb = qRgb(R,G,B);
                }

                m_satellite_image.setPixel(x,y,rgb);
                //qDebug()<<"R G B: "<<R<<G<<B;
            }

        }
    }
    //scene->clear();
    auto pixmap = QPixmap::fromImage(m_satellite_image);
    m_image_item = new QGraphicsPixmapItem(pixmap);
    m_image_item->setCursor(Qt::CrossCursor);
    m_image_item->setZValue(0);
    scene->addItem(m_image_item);
    scene->setSceneRect(pixmap.rect());
    ui->graphicsView_satellite_image->centerOn(m_image_item);
}
