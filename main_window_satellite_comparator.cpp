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
#include "dynamic_checkbox_widget.h"


uchar *raster_char;

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , m_sat_comparator(new SatteliteComparator)
{
    ui->setupUi(this);
    ui->label_satellite_image->setScaledContents(true);
    qDebug()<<m_sat_comparator->get_satellites_data().keys();
}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
}

void MainWindowSatelliteComparator::openHeaderData()
{
    QString headerName =  QFileDialog::getOpenFileName(this, "Открыть файл _MTL.json","",
                                                       "JSON и XML файлы(*_MTL.xml *_MTL.json *.TIF)");
    QFile file(headerName);
    if(file.exists()==false)return;
    QJsonObject jo;
    jsn::getJsonObjectFromFile(headerName,jo);
    QJsonDocument jsonDoc(jo);
    //ui->textBrowser_header_info->setText(jsonDoc.toJson(QJsonDocument::Indented));

    if(jo.contains("LANDSAT_METADATA_FILE")){
      QJsonValue value = jsn::getValueByPath(jo,{"LANDSAT_METADATA_FILE","PRODUCT_CONTENTS"});
      QJsonObject check_bands = value.toObject();
      //qDebug()<<"bands: "<<jsn::toString(check_bands);
      const char temp_str[] = "FILE_NAME_BAND_%1";
      for(int i=1;i<12;++i){
          m_band_names.append(check_bands[QString(temp_str).arg(i)].toString());
      }
    };
    DynamicCheckboxWidget *widget = new DynamicCheckboxWidget(m_band_names,
                                                              this,
                                                              ui->verticalLayout_bands);
    connect(widget,SIGNAL(choosed_bands_changed()),this,SLOT(change_bands_and_show_image()));
    QFileInfo fi(headerName);
    qDebug()<<"JUST FILE PATH: "<<fi.path();
    m_root_path = fi.path();
    for(int i=1;i<4;++i){
    m_landsat8_bands[i] = readTiff(fi.path()+"/"+m_band_names[i]);
    }

    const int nXSize = 7681;
    const int nYSize = 7781;
    m_satellite_image = QImage(nXSize, nYSize, QImage::QImage::Format_RGB888);
    for (int y = 0; y < nYSize; ++y) {
        for (int x = 0; x < nXSize; ++x) {

            int B = static_cast<int>(m_landsat8_bands[1][y * nXSize + x] / 255.0)*3;
            int G = static_cast<int>(m_landsat8_bands[2][y * nXSize + x] / 255.0)*3;
            int R = static_cast<int>(m_landsat8_bands[3][y * nXSize + x] / 255.0)*3;
            QRgb rgb(qRgb(R,G,B));
            m_satellite_image.setPixel(x,y,rgb);

        }
    }
    ui->label_satellite_image->setPixmap(QPixmap::fromImage(m_satellite_image));
}

uint16_t *MainWindowSatelliteComparator::readTiff(const QString& path)
{
    QString imgPath = path;
    QByteArray ba = imgPath.toUtf8();
    QFile file(imgPath);
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    bool isFileExists = file.exists();
    qDebug()<<"File exists: -->"<<isFileExists;
    if(false==isFileExists)return nullptr;
    const char *fileName = ba.constData();
    GDALAllRegister();
    GDALDataset* poDataset = (GDALDataset*) GDALOpen( fileName, GA_ReadOnly );

    GDALRasterBand  *poBand;
    int             nBlockXSize, nBlockYSize;
    poBand = poDataset->GetRasterBand(1);
    poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
    int   nXSize = poBand->GetXSize();
    int   nYSize = poBand->GetYSize();
    uint16_t* raster = new uint16[nXSize*nYSize];

    poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, raster, nXSize, nYSize, GDT_UInt16, 0, 0);

    //qDebug()<<"NORMALIZED VALUE: "<<((float)raster_char[(nYSize*nXSize)/2]/max_value)*255;
    qDebug()<<poBand->GetXSize()<<poBand->GetYSize();
    GDALClose(poDataset);
    GDALDestroyDriverManager();
    //ui->label_satellite_image->setPixmap(QPixmap::fromImage(m_satellite_image));
    return raster;

}


void MainWindowSatelliteComparator::on_pushButton_open_sat_header_clicked()
{
    openHeaderData();
}

void MainWindowSatelliteComparator::change_bands_and_show_image()
{
    qDebug()<<"change bands slot";
}
