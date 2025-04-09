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

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowSatelliteComparator)
    , m_sat_comparator(new SatteliteComparator)
{
    ui->setupUi(this);
    qDebug()<<m_sat_comparator->get_satellites_data().keys();
}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator()
{
    delete ui;
}

void MainWindowSatelliteComparator::openHeaderData()
{
    /*QString headerName =  QFileDialog::getOpenFileName(this, "Открыть файл _MTL.json","",
                                                       "JSON и XML файлы(*.xml *.json)");
    if(QFile::exists(headerName)==false)return;*/
    readTiff();

}

void MainWindowSatelliteComparator::readTiff()
{
    QString imgPath ="V:/Наследство Ольги Красовской (Силюк)/Спутниковые данные/Landsat8 Gobabeb/LC08_L1TP_179076_20230719_20230802_02_T1/LC08_L1TP_179076_20230719_20230802_02_T1_B2.TIF";
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
        poBand = poDataset->GetRasterBand(1);//каналы начинаются с 1
        poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
        int   nXSize = poBand->GetXSize();
        int   nYSize = poBand->GetYSize();
        float *raster = new float[nXSize*nYSize];
        poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, raster, nXSize, nYSize, GDT_UInt16, 0, 0);

        for (int i = 0; i < nYSize*nXSize; ++i)
        {
            raster[i] = raster[i]*0.0001 + 0;
        }
        qDebug()<<poBand->GetXSize()<<poBand->GetYSize();

}


void MainWindowSatelliteComparator::on_pushButton_open_sat_header_clicked()
{
    openHeaderData();
}
