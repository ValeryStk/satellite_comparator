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



uchar *raster_char;

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , m_sat_comparator(new SatteliteComparator)
{
    ui->setupUi(this);
    ui->label_satellite_image->setScaledContents(true);
    //qDebug()<<m_sat_comparator->get_satellites_data().keys();
    m_is_image_created = false;
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
    if(file.exists()==false)return;
    QJsonObject jo;
    jsn::getJsonObjectFromFile(headerName,jo);

    QList<QString> landsat_bands_ranges;
    if(jo.contains("LANDSAT_METADATA_FILE")){
        QJsonValue value = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","PRODUCT_CONTENTS"});
        QJsonObject check_bands = value.toObject();
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
        for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
            m_landsat8_bands_file_names[i] = check_bands[m_landsat8_bands_keys[i]].toString();
        }
    }else{
        return;// Пока работает только LANDSAT
    };
    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(landsat_bands_ranges,
                                                            ui->verticalLayout_bands);
    connect(m_dynamic_checkboxes_widget,SIGNAL(choosed_bands_changed()),this,SLOT(change_bands_and_show_image()));
    QFileInfo fi(headerName);
    m_root_path = fi.path();

    for(int i=0;i<LANDSAT_BANDS_NUMBER;++i){
        int xS;
        int yS;
        m_landsat8_bands[i] = readTiff(fi.path()+"/"+m_landsat8_bands_file_names[i],xS,yS);
        m_landsat8_bands_image_sizes[i] = {xS,yS};
    }
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1,2,3});
    change_bands_and_show_image();
    m_is_image_created = true;
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


void MainWindowSatelliteComparator::on_pushButton_open_sat_header_clicked()
{
    openHeaderData();
}

void MainWindowSatelliteComparator::change_bands_and_show_image()
{
    //qDebug()<<"change bands slot";
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
                    B = static_cast<int>(m_landsat8_bands[bands[j].first][y * nXSize + x] / 255.0)*3;
                }else if(bands[j].second==GREEN){
                    G = static_cast<int>(m_landsat8_bands[bands[j].first][y * nXSize + x] / 255.0)*3;
                }else if(bands[j].second==RED){
                    R = static_cast<int>(m_landsat8_bands[bands[j].first][y * nXSize + x] / 255.0)*3;
                }
                QRgb rgb(qRgb(R,G,B));
                m_satellite_image.setPixel(x,y,rgb);
            }

        }
    }
    ui->label_satellite_image->setPixmap(QPixmap::fromImage(m_satellite_image));
}
