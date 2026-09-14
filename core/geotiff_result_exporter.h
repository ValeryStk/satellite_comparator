#ifndef GEOTIFF_RESULT_EXPORTER_H
#define GEOTIFF_RESULT_EXPORTER_H

#include <QColor>
#include <QImage>
#include <QPair>
#include <QRect>
#include <QString>
#include <QVector>
#include <cstdint>

class QGraphicsPixmapItem;
class QWidget;

namespace sad {
struct geoTransform;
}

// Явная, опциональная легенда "цвет -> подпись", которую вызывающий код
// (MainWindowSatelliteComparator) передаёт в экспортёр для тех результатов,
// у которых есть фиксированный, заранее известный набор классов (например,
// градиентная DP-маска усыхания: "DP 0", "I".."V" - см. dpClassColor()).
//
// Экспортёр НИЧЕГО не знает о конкретных цветах и их смысле - он только
// сверяет цвет, реально найденный в изображении, с этим списком (если он
// передан) и берёт оттуда подпись. Если список пуст или совпадения нет -
// используется generic "Класс N". Это специально избавляет экспортёр от
// зашитых внутрь цветовых констант: если палитра dpClassColor() когда-нибудь
// изменится, достаточно поправить её один раз в месте построения легенды,
// а не искать зависимость внутри экспортёра.
using GeoTiffClassLegend = QVector<QPair<QColor, QString>>;

// Экспорт одного результата из панели "Результаты поиска" (градиентная маска
// усыхания, поиск по метрике, MATLAB-классификация) в тематический GeoTIFF.
//
// Идея: все три источника результата в приложении уже приводятся к одному
// виду - QGraphicsPixmapItem с RGBA8888 изображением того же размера, что и
// базовый снимок, где alpha == 0 означает "нет данных" (см. main_window_
// satellite_comparator.cpp: buildGradientMaskItem, paintSamplePoints,
// paintMultiSpecPoints, paintTimeRowBadForest). Поэтому экспортёр не должен
// знать, откуда взялся результат - ему достаточно самого QImage и,
// опционально, явной легенды для подписей классов.
//
// Алгоритм:
//   1. Сканируем непрозрачные пиксели, собираем уникальные RGB-цвета.
//   2. Каждому уникальному цвету присваиваем код класса 1..N (0 зарезервирован
//      под NoData - под него как раз попадают все alpha == 0 пиксели).
//   3. Обрезаем растр по фактическому bounding box содержимого (без сплошных
//      полей NoData по краям, которые остаются от размера всего снимка) и
//      соответствующим образом сдвигаем geoTransform.
//   4. Пишем однослойный GDT_Byte GeoTIFF: пиксель = код класса, NoData = 0.
//   5. В тот же файл встраиваем GDALColorTable (чтобы ГИС сразу красиво
//      отрисовала слой) и GDALRasterAttributeTable с подписями классов
//      (из переданной легенды, либо "Класс N").
class GeoTiffResultExporter {
public:
    struct ExportOptions {
        bool exportSubstrate =
            false;  // сохранить RGB-подложку отдельным файлом
        bool exportLegendPng = false;  // сохранить легенду отдельной картинкой
        bool openFolderAfterSave =
            true;  // открыть папку с результатом по завершении
    };

    // resultItem    - маска результата (уже отрисованный RGBA
    // QGraphicsPixmapItem
    //                 из m_layers_search_result_items)
    // suggestedName - имя по умолчанию для диалога сохранения (обычно id
    // результата
    //                 из списка "Результаты поиска"). Может содержать символы,
    //                 недопустимые в именах файлов - они будут заменены на "_".
    // geoTransform  - геопривязка базового снимка. Для обычного режима это
    // m_geo,
    //                 для режима временного ряда - m_time_row_geo[0]
    //                 (см. существующий пример выбора в setCursorByGeo)
    // baseImage     - базовый RGB снимок (m_satellite_image), нужен только если
    //                 options.exportSubstrate == true; можно передать пустой
    //                 QImage, если подложка не нужна
    // knownLegend   - опциональная явная легенда "цвет -> подпись" для
    // результатов
    //                 с фиксированным набором классов. Для результатов без
    //                 такой легенды передайте пустой GeoTiffClassLegend() -
    //                 будет использована generic-нумерация "Класс N"
    // parent        - родительский виджет для диалогов сохранения/ошибок
    //
    // Возвращает true, если основной GeoTIFF с маской успешно записан
    // (подложка и легенда - не критичны для итогового результата "true").
    static bool exportSearchResult(QGraphicsPixmapItem *resultItem,
                                   const QString &suggestedName,
                                   const sad::geoTransform &geoTransform,
                                   const QImage &baseImage,
                                   const ExportOptions &options,
                                   const GeoTiffClassLegend &knownLegend,
                                   QWidget *parent);

private:
    struct ClassRaster {
        QVector<uint8_t> values;  // xSize*ySize, 0 = NoData, 1..N = код класса
        QVector<QRgb> palette;  // palette[classCode - 1] = исходный RGB-цвет
        int xSize = 0;
        int ySize = 0;
    };

    // Заменяет символы, недопустимые в именах файлов Windows/Linux
    // (\ / : * ? " < > |), а также пробелы по краям, на "_".
    static QString sanitizeFileName(const QString &name);

    static ClassRaster buildClassRaster(const QImage &rgbaImage);

    // Обрезает raster по прямоугольнику фактического содержимого (пиксели с
    // ненулевым кодом класса), сдвигает geo на смещение этого прямоугольника.
    // cropRectOut - тот же прямоугольник в исходных (некадрированных)
    // пиксельных координатах, нужен, чтобы так же обрезать подложку.
    // Возвращает false, если во всём растре нет ни одного валидного пикселя.
    static bool cropToContent(ClassRaster &raster, sad::geoTransform &geo,
                              QRect &cropRectOut);

    // Возвращает подпись для данного цвета: сначала ищет точное совпадение
    // в knownLegend, если не находит - "Класс classIndex".
    static QString classLabelFor(QRgb color, int classIndex,
                                 const GeoTiffClassLegend &knownLegend);

    static bool writeClassGeoTiff(const QString &filePath,
                                  const ClassRaster &raster,
                                  const sad::geoTransform &geo,
                                  const GeoTiffClassLegend &knownLegend);

    static bool writeSubstrateGeoTiff(const QString &filePath,
                                      const QImage &baseImage,
                                      const QRect &cropRect,
                                      const sad::geoTransform &geo);

    static bool writeLegendPng(const QString &filePath,
                               const ClassRaster &raster,
                               const GeoTiffClassLegend &knownLegend);
};

#endif  // GEOTIFF_RESULT_EXPORTER_H
