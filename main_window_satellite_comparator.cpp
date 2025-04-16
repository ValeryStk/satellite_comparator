#include "main_window_satellite_comparator.h"
#include "ui_main_window_satellite_comparator.h"
#include <algorithm>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include "libs/gdal/x64/include/gdal_priv.h"
#include "libs/gdal/x64/include/cpl_conv.h" // for CPLMalloc()
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
                                                       "JSON и XML файлы(*.xml *.json *.TIF)");
    if(QFile::exists(headerName)==false)return;
    QJsonObject jo;
    jsn::getJsonObjectFromFile(headerName,jo);
    QJsonDocument jsonDoc(jo);
    ui->textBrowser_header_info->setText(jsonDoc.toJson(QJsonDocument::Indented));
    if(jo.contains("LANDSAT_METADATA_FILE")){
      QJsonObject check_bands = jo["LANDSAT_METADATA_FILE"].toObject()["PRODUCT_CONTENTS"].toObject();
      qDebug()<<"bands: "<<jsn::toString(check_bands);
      const char temp_str[] = "FILE_NAME_BAND_%1";
      for(int i=1;i<12;++i){
          qDebug()<<QString(temp_str).arg(i);
          qDebug()<<check_bands[QString(temp_str).arg(i)].toString();
      }
    };
    //readTiff(headerName);
}

void MainWindowSatelliteComparator::readTiff(const QString& path)
{
    QString imgPath = path;
    QByteArray ba = imgPath.toUtf8();
    QFile file(imgPath);
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    bool isFileExists = file.exists();
    qDebug()<<"File exists: -->"<<isFileExists;
    if(false==isFileExists)return;
    const char *fileName = ba.constData();
    GDALAllRegister();//register all known drivers
    GDALDataset* poDataset = (GDALDataset*) GDALOpen( fileName, GA_ReadOnly );

    GDALRasterBand  *poBand;
    int             nBlockXSize, nBlockYSize;
    poBand = poDataset->GetRasterBand(1);
    poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
    int   nXSize = poBand->GetXSize();
    int   nYSize = poBand->GetYSize();
    uint16 *raster = new uint16[nXSize*nYSize];

    poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, raster, nXSize, nYSize, GDT_UInt16, 0, 0);
    QImage image(nXSize, nYSize, QImage::QImage::Format_Indexed8);

    for (int y = 0; y < nYSize; ++y) {
        uchar *scanline = image.scanLine(y);
        for (int x = 0; x < nXSize; ++x) {
            //scanline[x] = raster[y * nXSize + x];
            scanline[x] = static_cast<uchar>(raster[y * nXSize + x] / 256);

        }
    }


    //qDebug()<<"NORMALIZED VALUE: "<<((float)raster_char[(nYSize*nXSize)/2]/max_value)*255;
    qDebug()<<poBand->GetXSize()<<poBand->GetYSize();

    ui->label_satellite_image->setPixmap(QPixmap::fromImage(image));

}


void MainWindowSatelliteComparator::on_pushButton_open_sat_header_clicked()
{
    openHeaderData();
}
