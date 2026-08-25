#include "geotiff_result_exporter.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QHash>
#include <QMessageBox>
#include <QPainter>
#include <QUrl>

#include "libs/gdal/x64/include/cpl_conv.h"
#include "libs/gdal/x64/include/gdal_priv.h"
#include "libs/gdal/x64/include/gdal_rat.h"
#include "libs/gdal/x64/include/ogr_spatialref.h"
#include "satellites_structs.h"  // sad::geoTransform (ulX, ulY, resX, resY, utmZone, isNorth)

QString GeoTiffResultExporter::sanitizeFileName(const QString &name) {
    QString result = name.trimmed();

    // Символы, недопустимые в именах файлов Windows (и часть из них - в Linux):
    // \ / : * ? " < > |
    static const QString forbidden = "\\/:*?\"<>|";
    for (const QChar &ch : forbidden) {
        result.replace(ch, '_');
    }

    // На всякий случай схлопываем последовательные "_", образовавшиеся,
    // например, из "12:05:30" -> "12_05_30" -> уже нормально, а вот из
    // "id: something" -> "id_ something" -> "id__something" тоже приемлемо,
    // но сделаем аккуратнее.
    while (result.contains("__")) {
        result.replace("__", "_");
    }

    result = result.trimmed();
    if (result.isEmpty()) {
        result = "result";
    }
    return result;
}

GeoTiffResultExporter::ClassRaster GeoTiffResultExporter::buildClassRaster(
    const QImage &rgbaImageIn) {
    ClassRaster result;

    const QImage rgbaImage =
        (rgbaImageIn.format() == QImage::Format_RGBA8888)
            ? rgbaImageIn
            : rgbaImageIn.convertToFormat(QImage::Format_RGBA8888);

    result.xSize = rgbaImage.width();
    result.ySize = rgbaImage.height();
    if (result.xSize <= 0 || result.ySize <= 0) return result;

    result.values.fill(0, result.xSize * result.ySize);

    // Ключ - цвет без альфы. Класс 0 зарезервирован под NoData,
    // поэтому реальные классы нумеруются с 1.
    QHash<QRgb, int> colorToClass;

    for (int y = 0; y < result.ySize; ++y) {
        const uchar *line = rgbaImage.constScanLine(y);
        for (int x = 0; x < result.xSize; ++x) {
            const int px = x * 4;
            const uchar r = line[px + 0];
            const uchar g = line[px + 1];
            const uchar b = line[px + 2];
            const uchar a = line[px + 3];
            const int idx = y * result.xSize + x;

            if (a == 0) {
                result.values[idx] = 0;  // NoData
                continue;
            }

            const QRgb key = qRgb(r, g, b);
            int classCode = colorToClass.value(key, -1);
            if (classCode == -1) {
                if (colorToClass.size() >= 254) {
                    // Более 254 уникальных цветов в одной маске не ожидается
                    // (обычно до десятка классов). Подстраховка от
                    // переполнения.
                    result.values[idx] = 0;
                    continue;
                }
                classCode = colorToClass.size() + 1;
                colorToClass.insert(key, classCode);
                result.palette.append(key);
            }
            result.values[idx] = static_cast<uint8_t>(classCode);
        }
    }

    return result;
}

bool GeoTiffResultExporter::cropToContent(ClassRaster &raster,
                                          sad::geoTransform &geo,
                                          QRect &cropRectOut) {
    int minX = raster.xSize;
    int minY = raster.ySize;
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < raster.ySize; ++y) {
        const int rowOffset = y * raster.xSize;
        for (int x = 0; x < raster.xSize; ++x) {
            if (raster.values[rowOffset + x] != 0) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }

    if (maxX < minX || maxY < minY)
        return false;  // валидных пикселей нет вообще

    const int newW = maxX - minX + 1;
    const int newH = maxY - minY + 1;

    // Если содержимое и так занимает весь кадр - обрезка не нужна,
    // но всё равно возвращаем корректный cropRect для единообразия.
    if (newW != raster.xSize || newH != raster.ySize) {
        QVector<uint8_t> cropped(newW * newH, 0);
        for (int y = 0; y < newH; ++y) {
            const int srcRow = (y + minY) * raster.xSize;
            const int dstRow = y * newW;
            for (int x = 0; x < newW; ++x) {
                cropped[dstRow + x] = raster.values[srcRow + (x + minX)];
            }
        }
        raster.values = cropped;
        raster.xSize = newW;
        raster.ySize = newH;
    }

    // Сдвигаем геопривязку на смещение обрезанной области. resY обычно
    // отрицательный (см. mgeo.resY = -mgeo.resX в остальном коде проекта),
    // поэтому формула работает одинаково для обеих осей.
    geo.ulX += minX * geo.resX;
    geo.ulY += minY * geo.resY;

    cropRectOut = QRect(minX, minY, newW, newH);
    return true;
}

static void fillGdalGeoTransform(const sad::geoTransform &geo, double out[6]) {
    out[0] = geo.ulX;
    out[1] = geo.resX;
    out[2] = 0.0;
    out[3] = geo.ulY;
    out[4] = 0.0;
    out[5] = geo.resY;
}

static char *buildUtmWkt(const sad::geoTransform &geo) {
    OGRSpatialReference utmSrs;
    utmSrs.SetProjCS("UTM");
    utmSrs.SetWellKnownGeogCS("WGS84");
    utmSrs.SetUTM(static_cast<int>(geo.utmZone), geo.isNorth);

    char *wkt = nullptr;
    utmSrs.exportToWkt(&wkt);
    return wkt;  // освобождать через CPLFree
}

bool GeoTiffResultExporter::writeClassGeoTiff(const QString &filePath,
                                              const ClassRaster &raster,
                                              const sad::geoTransform &geo) {
    if (raster.xSize <= 0 || raster.ySize <= 0) return false;

    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver) return false;

    char **options = nullptr;
    options = CSLSetNameValue(options, "COMPRESS", "LZW");
    options = CSLSetNameValue(options, "TILED", "YES");

    GDALDataset *ds =
        driver->Create(filePath.toUtf8().constData(), raster.xSize,
                       raster.ySize, 1, GDT_Byte, options);
    CSLDestroy(options);
    if (!ds) return false;

    double geoTransformArr[6];
    fillGdalGeoTransform(geo, geoTransformArr);
    ds->SetGeoTransform(geoTransformArr);

    char *wkt = buildUtmWkt(geo);
    ds->SetProjection(wkt);
    CPLFree(wkt);

    GDALRasterBand *band = ds->GetRasterBand(1);
    band->SetNoDataValue(0);
    band->RasterIO(GF_Write, 0, 0, raster.xSize, raster.ySize,
                   const_cast<uint8_t *>(raster.values.constData()),
                   raster.xSize, raster.ySize, GDT_Byte, 0, 0);

    // Палитра - чтобы слой сразу правильно отрисовался в любой ГИС
    GDALColorTable colorTable(GPI_RGB);
    GDALColorEntry noDataEntry = {0, 0, 0, 0};
    colorTable.SetColorEntry(0, &noDataEntry);
    for (int i = 0; i < raster.palette.size(); ++i) {
        const QRgb c = raster.palette.at(i);
        GDALColorEntry entry = {static_cast<short>(qRed(c)),
                                static_cast<short>(qGreen(c)),
                                static_cast<short>(qBlue(c)), 255};
        colorTable.SetColorEntry(i + 1, &entry);
    }
    band->SetColorTable(&colorTable);
    band->SetColorInterpretation(GCI_PaletteIndex);

    // Таблица атрибутов растра (RAT) - чтобы в ГИС были подписи классов,
    // а не голые числа 1..N
    GDALDefaultRasterAttributeTable rat;
    rat.CreateColumn("Value", GFT_Integer, GFU_MinMax);
    rat.CreateColumn("ClassName", GFT_String, GFU_Name);
    rat.CreateColumn("Red", GFT_Integer, GFU_Red);
    rat.CreateColumn("Green", GFT_Integer, GFU_Green);
    rat.CreateColumn("Blue", GFT_Integer, GFU_Blue);

    rat.SetRowCount(raster.palette.size() + 1);
    rat.SetValue(0, 0, 0);
    rat.SetValue(0, 1, "NoData");
    rat.SetValue(0, 2, 0);
    rat.SetValue(0, 3, 0);
    rat.SetValue(0, 4, 0);

    for (int i = 0; i < raster.palette.size(); ++i) {
        const QRgb c = raster.palette.at(i);
        const int row = i + 1;
        rat.SetValue(row, 0, i + 1);
        rat.SetValue(row, 1,
                     QString("Класс %1").arg(i + 1).toUtf8().constData());
        rat.SetValue(row, 2, qRed(c));
        rat.SetValue(row, 3, qGreen(c));
        rat.SetValue(row, 4, qBlue(c));
    }
    band->SetDefaultRAT(&rat);

    GDALClose(ds);
    return true;
}

bool GeoTiffResultExporter::writeSubstrateGeoTiff(
    const QString &filePath, const QImage &baseImageIn, const QRect &cropRect,
    const sad::geoTransform &geo) {
    const QImage baseImage =
        (baseImageIn.format() == QImage::Format_RGB888)
            ? baseImageIn
            : baseImageIn.convertToFormat(QImage::Format_RGB888);

    if (cropRect.width() <= 0 || cropRect.height() <= 0) return false;
    if (cropRect.right() >= baseImage.width() ||
        cropRect.bottom() >= baseImage.height()) {
        // Подложка не совпадает по размеру с маской (например, базовый снимок
        // сменился после расчёта результата) - пропускаем, чтобы не писать
        // рассинхронизированный файл.
        return false;
    }

    const int xSize = cropRect.width();
    const int ySize = cropRect.height();

    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver) return false;

    char **options = nullptr;
    options = CSLSetNameValue(options, "COMPRESS", "LZW");
    options = CSLSetNameValue(options, "TILED", "YES");

    GDALDataset *ds = driver->Create(filePath.toUtf8().constData(), xSize,
                                     ySize, 3, GDT_Byte, options);
    CSLDestroy(options);
    if (!ds) return false;

    double geoTransformArr[6];
    fillGdalGeoTransform(geo, geoTransformArr);
    ds->SetGeoTransform(geoTransformArr);

    char *wkt = buildUtmWkt(geo);
    ds->SetProjection(wkt);
    CPLFree(wkt);

    QVector<uchar> channelBuf(xSize * ySize);
    for (int bandIdx = 0; bandIdx < 3; ++bandIdx) {
        for (int y = 0; y < ySize; ++y) {
            const uchar *line = baseImage.constScanLine(cropRect.top() + y);
            for (int x = 0; x < xSize; ++x) {
                channelBuf[y * xSize + x] =
                    line[(cropRect.left() + x) * 3 + bandIdx];
            }
        }
        ds->GetRasterBand(bandIdx + 1)
            ->RasterIO(GF_Write, 0, 0, xSize, ySize, channelBuf.data(), xSize,
                       ySize, GDT_Byte, 0, 0);
    }

    GDALClose(ds);
    return true;
}

bool GeoTiffResultExporter::writeLegendPng(const QString &filePath,
                                           const ClassRaster &raster) {
    if (raster.palette.isEmpty()) return false;

    const int swatch = 24;
    const int rowH = 30;
    const int width = 260;
    const int height = rowH * raster.palette.size() + 10;

    QImage legend(width, height, QImage::Format_RGB888);
    legend.fill(Qt::white);

    QPainter painter(&legend);
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    for (int i = 0; i < raster.palette.size(); ++i) {
        const int y = 5 + i * rowH;
        const QRgb c = raster.palette.at(i);
        painter.fillRect(10, y, swatch, swatch, QColor(c));
        painter.drawRect(10, y, swatch, swatch);
        painter.drawText(10 + swatch + 10, y + swatch - 6,
                         QString("Класс %1").arg(i + 1));
    }
    painter.end();

    return legend.save(filePath, "PNG");
}

bool GeoTiffResultExporter::exportSearchResult(
    QGraphicsPixmapItem *resultItem, const QString &suggestedName,
    const sad::geoTransform &geoTransform, const QImage &baseImage,
    const ExportOptions &options, QWidget *parent) {
    if (!resultItem) {
        QMessageBox::warning(parent, "Экспорт", "Результат не найден.");
        return false;
    }

    const QString cleanName = sanitizeFileName(suggestedName);

    const QString filePath = QFileDialog::getSaveFileName(
        parent, "Экспорт результата в GeoTIFF", cleanName + ".tif",
        "GeoTIFF (*.tif *.tiff)");
    if (filePath.isEmpty()) return false;

    const QImage maskImage = resultItem->pixmap().toImage();
    ClassRaster raster = buildClassRaster(maskImage);
    if (raster.xSize <= 0) {
        QMessageBox::warning(parent, "Экспорт",
                             "Не удалось прочитать данные результата.");
        return false;
    }

    sad::geoTransform croppedGeo =
        geoTransform;  // локальная копия - оригинал не трогаем
    QRect cropRect(0, 0, raster.xSize, raster.ySize);
    if (!cropToContent(raster, croppedGeo, cropRect)) {
        QMessageBox::warning(
            parent, "Экспорт",
            "В этом результате нет валидных данных для экспорта.");
        return false;
    }

    if (!writeClassGeoTiff(filePath, raster, croppedGeo)) {
        QMessageBox::warning(parent, "Экспорт", "Не удалось записать GeoTIFF.");
        return false;
    }

    const QFileInfo fi(filePath);
    const QString baseNameNoExt = fi.completeBaseName();
    const QString dirPath = fi.absolutePath();

    if (options.exportSubstrate && !baseImage.isNull()) {
        const QString substratePath =
            dirPath + "/" + baseNameNoExt + "_substrate.tif";
        writeSubstrateGeoTiff(substratePath, baseImage, cropRect, croppedGeo);
    }

    if (options.exportLegendPng) {
        const QString legendPath =
            dirPath + "/" + baseNameNoExt + "_legend.png";
        writeLegendPng(legendPath, raster);
    }

    if (options.openFolderAfterSave) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    }

    return true;
}
