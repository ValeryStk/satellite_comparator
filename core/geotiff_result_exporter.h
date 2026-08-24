#ifndef GEOTIFF_RESULT_EXPORTER_H
#define GEOTIFF_RESULT_EXPORTER_H

#include <QImage>
#include <QString>
#include <QVector>
#include <cstdint>

class QGraphicsPixmapItem;
class QWidget;

namespace sad {
struct geoTransform;
}

// Экспорт одного результата из панели "Результаты поиска" (градиентная маска
// усыхания, поиск по метрике, MATLAB-классификация) в тематический GeoTIFF.
//
// Идея: все три источника результата в приложении уже приводятся к одному
// виду - QGraphicsPixmapItem с RGBA8888 изображением того же размера, что и
// базовый снимок, где alpha == 0 означает "нет данных" (см. main_window_
// satellite_comparator.cpp: buildGradientMaskItem, paintSamplePoints,
// paintMultiSpecPoints, paintTimeRowBadForest). Поэтому экспортёр не должен
// знать, откуда взялся результат - ему достаточно самого QImage.
//
// Алгоритм:
//   1. Сканируем непрозрачные пиксели, собираем уникальные RGB-цвета.
//   2. Каждому уникальному цвету присваиваем код класса 1..N (0 зарезервирован
//      под NoData - под него как раз попадают все alpha == 0 пиксели).
//   3. Пишем однослойный GDT_Byte GeoTIFF: пиксель = код класса, NoData = 0.
//   4. В тот же файл встраиваем GDALColorTable (чтобы ГИС сразу красиво
//      отрисовала слой) и GDALRasterAttributeTable (чтобы в ГИС были видны
//      подписи классов, а не голые числа).
class GeoTiffResultExporter {
public:
    struct ExportOptions {
        bool exportSubstrate = false;    // сохранить RGB-подложку отдельным файлом
        bool exportLegendPng = false;    // сохранить легенду отдельной картинкой
        bool openFolderAfterSave = true; // открыть папку с результатом по завершении
    };

    // resultItem    - маска результата (уже отрисованный RGBA QGraphicsPixmapItem
    //                 из m_layers_search_result_items)
    // suggestedName - имя по умолчанию для диалога сохранения (обычно id результата
    //                 из списка "Результаты поиска")
    // geoTransform  - геопривязка базового снимка. Для обычного режима это m_geo,
    //                 для режима временного ряда - m_time_row_geo[0]
    //                 (см. существующий пример выбора в setCursorByGeo)
    // baseImage     - базовый RGB снимок (m_satellite_image), нужен только если
    //                 options.exportSubstrate == true; можно передать пустой QImage,
    //                 если подложка не нужна
    // parent        - родительский виджет для диалогов сохранения/ошибок
    //
    // Возвращает true, если основной GeoTIFF с маской успешно записан
    // (подложка и легенда - не критичны для итогового результата "true").
    static bool exportSearchResult(QGraphicsPixmapItem *resultItem,
                                    const QString &suggestedName,
                                    const sad::geoTransform &geoTransform,
                                    const QImage &baseImage,
                                    const ExportOptions &options,
                                    QWidget *parent);

private:
    struct ClassRaster {
        QVector<uint8_t> values; // xSize*ySize, 0 = NoData, 1..N = код класса
        QVector<QRgb> palette;   // palette[classCode - 1] = исходный RGB-цвет
        int xSize = 0;
        int ySize = 0;
    };

    static ClassRaster buildClassRaster(const QImage &rgbaImage);

    static bool writeClassGeoTiff(const QString &filePath,
                                   const ClassRaster &raster,
                                   const sad::geoTransform &geo);

    static bool writeSubstrateGeoTiff(const QString &filePath,
                                       const QImage &baseImage,
                                       const sad::geoTransform &geo);

    static bool writeLegendPng(const QString &filePath,
                                const ClassRaster &raster);
};

#endif // GEOTIFF_RESULT_EXPORTER_H
