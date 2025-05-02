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




uchar *raster_char;
QGraphicsScene *scene;

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , m_sat_comparator(new SatteliteComparator)
{
    ui->setupUi(this);
    scene = new QGraphicsScene;
    connect(ui->graphicsView_satellite_image,&SatelliteGraphicsView::pointChanged,[this](QPointF pos){
        QString message = "x: ";
        message.append(QString::number(pos.x()));
        message.append(" y :");
        message.append(QString::number(pos.y()));
        ui->statusbar->showMessage(message);
    });
    ui->graphicsView_satellite_image->setMouseTracking(true);
    ui->graphicsView_satellite_image->setScene(scene);
    m_is_image_created = false;



    QWidget* widget_tools = new QWidget(ui->graphicsView_satellite_image);
    QHBoxLayout* toolLayOut = new QHBoxLayout;
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
    toolLayOut->addWidget(pushbutton_centerOn);
    toolLayOut->addWidget(zoomInButton);
    toolLayOut->addWidget(zoomOutButton);
    widget_tools->setLayout(toolLayOut);
    widget_tools->show();
    QGraphicsProxyWidget* proxy = scene->addWidget(widget_tools);
    proxy->setPos(0, 50);
    proxy->setGeometry(QRect(0,0,200,50));
    QObject::connect(zoomInButton, &QPushButton::clicked, [this]() {
        ui->graphicsView_satellite_image->scale(1.2, 1.2); // Увеличение масштаба
    });
    QObject::connect(zoomOutButton, &QPushButton::clicked, [this]() {
        ui->graphicsView_satellite_image->scale(0.8, 0.8); // Уменьшение масштаба
    });

    connect(pushbutton_centerOn,&QPushButton::clicked,[this](){ // Центрирование
        ui->graphicsView_satellite_image->centerOn(scene->sceneRect().center());

    });

}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
}

void MainWindowSatelliteComparator::openHeaderData()
{
    QString headerName =  QFileDialog::getOpenFileName(this, "Открыть файл _MTL.json","",
                                                       "JSON и XML файлы(*_MTL.txt *_MTL.xml *_MTL.json *.TIF)");
    QFile file(headerName);
    static bool isHeaderValid = false;
    if(file.exists()==false)return;
    QFileInfo fi(headerName);
    m_root_path = fi.path();
    const QString extension =fi.completeSuffix();


    if(extension == "json"){
        QJsonObject jo;
        jsn::getJsonObjectFromFile(headerName,jo);
        if(jo.contains("LANDSAT_METADATA_FILE")){
            QJsonValue value = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","PRODUCT_CONTENTS"});
            QJsonValue radiance_value = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","LEVEL1_RADIOMETRIC_RESCALING"});
            QJsonObject check_bands = value.toObject();
            QJsonObject radiance = radiance_value.toObject();
            //qDebug()<<"Radiance: "<<radiance;


            for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
                auto band_file_name = check_bands[m_landsat8_bands_keys[i]].toString();
                int xS;
                int yS;
                m_landsat8_data_bands[i] = readTiff(fi.path()+"/"+band_file_name,xS,yS);
                m_landsat8_bands_image_sizes[i] = {xS,yS};
                double mult_rad = radiance[m_landsat8_mult_radiance_keys[i]].toString().toDouble();
                double add_rad = radiance[m_landsat8_add_radiance_keys[i]].toString().toDouble();
                m_radiance_mult_add_arrays[i][0] = mult_rad;
                m_radiance_mult_add_arrays[i][1] = add_rad;
            }
            /*for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
              qDebug()<<"mult"<<i<<"mult"<<m_radiance_mult_add_arrays[i][0];
              qDebug()<<"add"<<i<<"add"<<m_radiance_mult_add_arrays[i][1];
            }*/
            isHeaderValid = true;
        }else{
            return;// Пока работает только LANDSAT
        };
    }else if(extension == "txt"){
        auto file_names = getLandSat8BandsFromTxtFormat(headerName);
        read_landsat_bands_data(file_names);
        isHeaderValid = true;
    }else{
        return;
    }
    if(isHeaderValid == false)return;


    QList<QString> landsat_bands_ranges;
    landsat_bands_ranges =             {"433 - 453 nm (Aerosol) 30 m",
                                        "450 - 515 nm (Blue) 30 m",
                                        "525 - 600 nm (Green) 30 m",
                                        "630 - 680 nm (Red) 30 m",
                                        "845 - 885 nm (Near infrared) 30 m",
                                        "1560 - 1660 nm (SWIR 1) 30 m",
                                        "2100 - 2300 nm (SWIR 2) 30 m",
                                        "500 - 680 nm (Panchromatic) 15 m",
                                        "1360 - 1390 nm (Cirrus) 30 m",
                                        "10300 - 11300 nm (LWIR) 100 m",
                                        "11500 - 12500 nm (LWIR) 100 m"
                                       };


    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(landsat_bands_ranges,
                                                            ui->verticalLayout_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1,2,3});
    connect(m_dynamic_checkboxes_widget,SIGNAL(choosed_bands_changed()),this,SLOT(change_bands_and_show_image()));


    change_bands_and_show_image();
    m_is_image_created = true;
}

QStringList MainWindowSatelliteComparator::getLandSat8BandsFromTxtFormat(const QString& path)
{
    QStringList bands;
    QFile file(path);
    if(file.exists()==false)return bands;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if(file.open(QIODevice::ReadOnly)==false)return bands;
    QString temp;
    while(ts.readLineInto(&temp)){
        if(temp.contains("FILE_NAME_BAND_")){
            temp = temp.mid(temp.indexOf('"'),temp.lastIndexOf('"'));
            temp.replace('"',"");
            bands.append(temp);
        }
        if(bands.size()==LANDSAT_BANDS_NUMBER)break;
    }
    return bands;
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
    if(file_names.size()!=LANDSAT_BANDS_NUMBER)return;
    for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
        auto band_file_name = file_names[i];
        int xS;
        int yS;
        m_landsat8_data_bands[i] = readTiff(m_root_path+"/"+band_file_name,xS,yS);
        m_landsat8_bands_image_sizes[i] = {xS,yS};
    }
}


void MainWindowSatelliteComparator::on_pushButton_open_sat_header_clicked()
{
    openHeaderData();
}

void MainWindowSatelliteComparator::change_bands_and_show_image()
{
    auto bands = m_dynamic_checkboxes_widget->get_choosed_bands();
    const int nXSize = m_landsat8_bands_image_sizes->first;
    const int nYSize = m_landsat8_bands_image_sizes->second;
    if(m_is_image_created==false){
        m_satellite_image = QImage(nXSize, nYSize, QImage::QImage::Format_RGB888);
    }
    for (int y = 0; y < nYSize; ++y) {
        for (int x = 0; x < nXSize; ++x) {
            int B = 0;
            int G = 0;
            int R = 0;
            for(int j=0;j<bands.size();++j){
                if(bands[j].second==BLUE){
                    B = static_cast<int>(m_landsat8_data_bands[bands[j].first][y * nXSize + x] / 255.0)*3;
                }else if(bands[j].second==GREEN){
                    G = static_cast<int>(m_landsat8_data_bands[bands[j].first][y * nXSize + x] / 255.0)*3;
                }else if(bands[j].second==RED){
                    R = static_cast<int>(m_landsat8_data_bands[bands[j].first][y * nXSize + x] / 255.0)*3;
                }
                QRgb rgb(qRgb(R,G,B));
                m_satellite_image.setPixel(x,y,rgb);
            }

        }
    }
    scene->clear();
    auto pixmap = QPixmap::fromImage(m_satellite_image);
    auto item = new QGraphicsPixmapItem(pixmap);
    scene->addItem(item);
    scene->setSceneRect(pixmap.rect());
    ui->graphicsView_satellite_image->centerOn(item);
}
