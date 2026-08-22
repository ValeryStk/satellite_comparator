#include "main_window_satellite_comparator.h"

#include <MatFilesOperator.h>
#include <matio.h>

#include <QByteArray>
#include <QDesktopServices>
#include <QDomDocument>
#include <QFile>
#include <QFileDialog>
#include <QShortcut>
#include <QSpacerItem>
#include <QTextCodec>
#include <QTextStream>
#include <QUrl>
#include <algorithm>
#include <limits>

#include "GeoPointFinder.h"
#include "MatFilesOperator.h"
#include "QApplication"
#include "QCheckBox"
#include "QDebug"
#include "QGraphicsPixmapItem"
#include "QGraphicsProxyWidget"
#include "QImageReader"
#include "cpl_conv.h"
#include "davis.h"
#include "google_maps_url_maker.h"
#include "health_ranges.h"
#include "icon_generator.h"
#include "image_utils.h"
#include "image_viewer.h"
#include "json_utils.h"
#include "layer_list.h"
#include "layer_roi_list.h"
#include "least_square_solver.h"
#include "libs/gdal/x64/include/cpl_conv.h"
#include "libs/gdal/x64/include/gdal_priv.h"
#include "libs/gdal/x64/include/ogr_spatialref.h"
#include "libs/gdal/x64/include/tiff.h"
#include "matlab_app_controller.h"
#include "progress_informator.h"
#include "rgb_stretch.h"
#include "sam.cpp"
#include "satellite_xml_reader.h"
#include "text_constants.h"
#include "ui_main_window_satellite_comparator.h"
#include "version.h"
#include "view_sync_manager.h"

class PixelGeo {
public:
    PixelGeo(sad::geoTransform geo) {
        m_geo = geo;
        // Создаем проекцию UTM
        utmSrs.SetProjCS("UTM");
        utmSrs.SetWellKnownGeogCS("WGS84");  // DATUM из MTL.json
        utmSrs.SetUTM(m_geo.utmZone,
                      true);  // Северное - true или южное - false полушарие

        // Создаем целевую проекцию (WGS84)
        wgs84Srs.SetWellKnownGeogCS("WGS84");

        // Создаем преобразователь координат
        transformer = OGRCreateCoordinateTransformation(&utmSrs, &wgs84Srs);
    }

    ~PixelGeo() { OCTDestroyCoordinateTransformation(transformer); }

    void getGeoCoordinates(const int x, const int y, double &latitude,
                           double &longitude) {
        // Вычисляем координаты в проекции UTM
        double utmX = m_geo.ulX + x * m_geo.resX + y * 0;
        double utmY = m_geo.ulY + x * 0 + y * m_geo.resY;

        // Преобразуем UTM -> WGS84 (широта/долгота)
        double lon = utmX;
        double lat = utmY;
        if (!transformer->Transform(1, &lon, &lat)) {
            latitude = 0;
            longitude = 0;
            return;
        }

        latitude = lat;
        longitude = lon;
    }

private:
    sad::geoTransform m_geo;
    OGRSpatialReference utmSrs;
    OGRSpatialReference wgs84Srs;
    OGRCoordinateTransformation *transformer;
};

PixelGeo *base_pixel_geo;

QCPTextElement *title_satellite_name;
QVector<double> waves_landsat9 = {443, 482, 562, 655, 865, 1610, 2200};
QVector<double> waves_landsat9_5 = {443, 482, 562, 655, 865};
QVector<double> waves_sentinel_2c = {444, 489, 560, 666,  707,  741, 785,
                                     835, 866, 947, 1372, 1612, 2191};
QVector<double> waves_sentinel_2c_5 = {444, 489, 560, 666, 707,
                                       741, 785, 835, 866, 947};

QList<QColor> distinctColors = {
    QColor(255, 0, 0),    // Красный
    QColor(0, 255, 0),    // Зеленый
    QColor(0, 0, 255),    // Синий
    QColor(255, 0, 255),  // Пурпурный
    QColor(0, 255, 255),  // Бирюзовый
    QColor(255, 165, 0),  // Оранжевый
    QColor(128, 0, 128),  // Фиолетовый
    QColor(0, 128, 128),  // Темно-бирюзовый
    QColor(128, 128, 0),  // Оливковый
    QColor(0, 0, 0),      // Чёрный
};

ViewSyncManager *syncManager;
QVector<ImageViewer *> m_viewers;

uint16_t *dataCloudMask = nullptr;
uint16_t *dataCloudMask2 = nullptr;

namespace {

QString getSclClassName(uint8_t classValue) {
    // static гарантирует инициализацию карты только при первом вызове функции
    static const QMap<uint8_t, QString> sclClasses = {
        {0, "No data"},
        {1, "Saturated or defective"},
        {2, "Dark area pixels"},
        {3, "Cloud shadows"},
        {4, "Vegetation"},
        {5, "Not vegetated"},
        {6, "Water"},
        {7, "Unclassified"},
        {8, "Cloud medium probability"},
        {9, "Cloud high probability"},
        {10, "Thin cirrus"},
        {11, "Snow or ice"}};

    // Возвращаем имя класса, либо "Unknown value", если код некорректен
    return sclClasses.value(classValue, "Unknown value");
}

QColor randomNiceColor() {
    int h = QRandomGenerator::global()->bounded(360);
    int s = QRandomGenerator::global()->bounded(160, 256);
    int v = QRandomGenerator::global()->bounded(180, 256);

    return QColor::fromHsv(h, s, v);
}

void downsample_uint16(const uint16_t *input, uint16_t *output, const int width,
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

void upsample_by_3_uint16(const uint16_t *input, uint16_t *output,
                          const int width, const int height) {
    int outWidth = width * 3;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t val = input[y * width + x];

            int outY = y * 3;
            int outX = x * 3;

            // 3×3 блок
            for (int dy = 0; dy < 3; ++dy) {
                for (int dx = 0; dx < 3; ++dx) {
                    output[(outY + dy) * outWidth + (outX + dx)] = val;
                }
            }
        }
    }
}

void copyQStringArray(const QString source[], QString destination[], int size) {
    for (int i = 0; i < size; ++i) {
        destination[i] = source[i];
    }
}

QStringList commonSortFilesByDateTime(const QStringList &unsortedFiles,
                                      const char splitter, const int index) {
    QStringList sortedScenes = unsortedFiles;

    std::sort(
        sortedScenes.begin(), sortedScenes.end(),
        [=](const QString &a, const QString &b) {
            QString dateStrA = a.split(splitter).value(index);
            QString dateStrB = b.split(splitter).value(index);
            QDate dateA = QDate::fromString(dateStrA.mid(0, 8), "yyyyMMdd");
            QDate dateB = QDate::fromString(dateStrB.mid(0, 8), "yyyyMMdd");
            return dateA < dateB;
        });
    return sortedScenes;
}

// Сортировка списка файлов Landsat по дате съёмки (4-й блок в имени файла)
QStringList sortLandsatFilesByDateTime(const QStringList &unsortedFiles) {
    return commonSortFilesByDateTime(unsortedFiles, '_', 3);
}

// Сортировка списка файлов Landsat по дате съёмки (4-й блок в имени файла)
QStringList sortSentinelFilesByDateTime(const QStringList &unsortedFiles) {
    return commonSortFilesByDateTime(unsortedFiles, '_', 2);
}

QString getPathToSentinelHeader(QWidget *context, const QString &satName) {
    qDebug() << satName;
    QString openSatMessage =
        QString("Открыть заголовочный файл %1").arg(satName);
    if (satName == satc::satellite_name_sentinel_2A_TOA) {
        qDebug() << "sentinel without atm correction file";
        return QFileDialog::getOpenFileName(context, openSatMessage, "",
                                            "файлы(MTD_MSIL1C.xml)");
    }
    return QFileDialog::getOpenFileName(context, openSatMessage, "",
                                        "файлы(MTD_MSIL2A.xml)");
}

sam::BandIndicesValues getBandsValues(const QVector<double> &waves,
                                      const QVector<double> &data,
                                      sad::SATELLITE_TYPE sat_type) {
    sam::BandIndicesValues biv;
    auto bi = sam::getBandsIndexes(sam::sk::SENTINEL);
    int blue_cw = 0;
    int green_cw = 0;
    int red_cw = 0;
    int nir1_cw = 0;
    int nir2_cw = 0;
    int swir1_cw = 0;
    int swir2_cw = 0;
    int swir3_cw = 0;

    switch (sat_type) {
        case sad::LANDSAT_9:
            break;
        case sad::LANDSAT_8:
            break;
        case sad::SENTINEL_2A:
            blue_cw = sad::sentinel_2A_central_wave_lengths[bi.blue];
            green_cw = sad::sentinel_2A_central_wave_lengths[bi.green];
            red_cw = sad::sentinel_2A_central_wave_lengths[bi.red];
            nir1_cw = sad::sentinel_2A_central_wave_lengths[bi.nir1];
            nir2_cw = sad::sentinel_2A_central_wave_lengths[bi.nir2];
            swir1_cw = sad::sentinel_2A_central_wave_lengths[bi.swir1];
            swir2_cw = sad::sentinel_2A_central_wave_lengths[bi.swir2];
            swir3_cw = sad::sentinel_2A_central_wave_lengths[bi.swir3];
            break;
        case sad::SENTINEL_2B:
            blue_cw = sad::sentinel_2B_central_wave_lengths[bi.blue];
            green_cw = sad::sentinel_2B_central_wave_lengths[bi.green];
            red_cw = sad::sentinel_2B_central_wave_lengths[bi.red];
            nir1_cw = sad::sentinel_2B_central_wave_lengths[bi.nir1];
            nir2_cw = sad::sentinel_2B_central_wave_lengths[bi.nir2];
            swir1_cw = sad::sentinel_2B_central_wave_lengths[bi.swir1];
            swir2_cw = sad::sentinel_2B_central_wave_lengths[bi.swir2];
            swir3_cw = sad::sentinel_2B_central_wave_lengths[bi.swir3];
            break;
        case sad::SENTINEL_2C:
            blue_cw = sad::sentinel_2C_central_wave_lengths[bi.blue];
            green_cw = sad::sentinel_2C_central_wave_lengths[bi.green];
            red_cw = sad::sentinel_2C_central_wave_lengths[bi.red];
            nir1_cw = sad::sentinel_2C_central_wave_lengths[bi.nir1];
            nir2_cw = sad::sentinel_2C_central_wave_lengths[bi.nir2];
            swir1_cw = sad::sentinel_2C_central_wave_lengths[bi.swir1];
            swir2_cw = sad::sentinel_2C_central_wave_lengths[bi.swir2];
            swir3_cw = sad::sentinel_2C_central_wave_lengths[bi.swir3];
            break;
        case sad::TIME_ROW_LANDSAT_COMBINATION:
            break;
        case sad::TIME_ROW_SENTINEL_COMBINATION:
            break;
        case sad::UNKNOWN_SATELLITE:
            break;
    }

    for (int i = 0; i < waves.size(); ++i) {
        if (waves[i] == blue_cw) {
            biv.blue = data[i];
        } else if (waves[i] == green_cw) {
            biv.green = data[i];
        } else if (waves[i] == red_cw) {
            biv.red = data[i];
        } else if (waves[i] == nir1_cw) {
            biv.nir1 = data[i];
        } else if (waves[i] == nir2_cw) {
            biv.nir2 = data[i];
        } else if (waves[i] == swir1_cw) {
            biv.swir1 = data[i];
        } else if (waves[i] == swir2_cw) {
            biv.swir2 = data[i];
        } else if (waves[i] == swir3_cw) {
            biv.swir3 = data[i];
        }
    }

    return biv;
}

void copyVectorsToClipboard(const double latitude, const double longitude,
                            const QVector<double> &col1,
                            const QVector<double> &col2) {
    if (col1.size() != col2.size()) {
        qWarning() << "Vectors have different sizes!";
        return;
    }

    QString text = QString("//latitude: %1\n//longitude: %2\n")
                       .arg(latitude)
                       .arg(longitude);
    for (int i = 0; i < col1.size(); ++i) {
        // Формат: значение1<TAB>значение2\n
        text +=
            QString::number(col1[i]) + " " + QString::number(col2[i]) + "\n";
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

}  // end of namespace

// ─── Вспомогательные структуры/функции ────────────────────────────────────

static QString buildIndexDynamicsLegendTooltip() {
    return "<b>Градиент изменений по индексам</b><br><br>"
           "<span style='color: rgb(0, 100, 120);'>■</span> Растет<br>"
           "<span style='color: rgb(34, 139, 34);'>■</span> I — "
           "Стабильно<br>"
           "<span style='color: rgb(120, 200, 80);'>■</span> II — Слабое "
           "ухудшение<br>"
           "<span style='color: rgb(255, 215, 0);'>■</span> III — Умеренное "
           "ухудшение<br>"
           "<span style='color: rgb(255, 140, 0);'>■</span> IV — Сильное "
           "ухудшение<br>"
           "<span style='color: rgb(165, 42, 42);'>■</span> V — Очень сильное "
           "ухудшение<br>";
}

static QString buildGradientLegendTooltipDP(const QColor &startColor,
                                            const QColor &endColor) {
    return QString(
               "<b>Градиент усыхания по NDVI (DP)</b><br><br>"
               "<span style='color: rgb(%1,%2,%3);'>■</span> DP = 0 — нет "
               "усыханий <br>"
               "<span style='color: rgb(%4,%5,%6);'>■</span> DP = 3 — усыхание "
               "в течение всего периода <br>")
        .arg(startColor.red())
        .arg(startColor.green())
        .arg(startColor.blue())
        .arg(endColor.red())
        .arg(endColor.green())
        .arg(endColor.blue());
}

struct GradientFitResult {
    double G = 0.0;   // наклон (Gradient)
    double b = 0.0;   // сдвиг
    double R2 = 0.0;  // коэффициент детерминации
    bool valid = false;
};

// Линейная регрессия y = G*x + b методом МНК
// x — Julian Days (double), y — значения NDVI или NDWI
static GradientFitResult fitLinear(const QVector<double> &x,
                                   const QVector<double> &y) {
    GradientFitResult res;
    int n = x.size();
    if (n < 3) return res;  // нужно минимум 3 точки

    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = 0; i < n; ++i) {
        sumX += x[i];
        sumY += y[i];
        sumXY += x[i] * y[i];
        sumX2 += x[i] * x[i];
    }
    double denom = n * sumX2 - sumX * sumX;
    if (qAbs(denom) < 1e-12) return res;

    res.G = (n * sumXY - sumX * sumY) / denom;
    res.b = (sumY - res.G * sumX) / n;

    // R²
    double meanY = sumY / n;
    double ssTot = 0, ssRes = 0;
    for (int i = 0; i < n; ++i) {
        double diff = y[i] - meanY;
        ssTot += diff * diff;
        double err = y[i] - (res.G * x[i] + res.b);
        ssRes += err * err;
    }
    res.R2 = (ssTot > 1e-12) ? (1.0 - ssRes / ssTot) : 0.0;
    res.valid = true;
    return res;
}

// Класс усыхания по градиенту NDWI согласно таблице из Технологической
// инструкции 3.2.1
static int classifyByNdwiGradient(double G, double R2) {
    if (R2 < 0.6) return -1;
    if (G > 0.00005) return 0;                    // Растет
    if (G >= -0.00005 && G <= 0.00005) return 1;  // I = Стабильно
    if (G >= -0.00010 && G < -0.00005) return 2;  // II
    if (G >= -0.00020 && G < -0.00010) return 3;  // III
    if (G >= -0.00040 && G < -0.00020) return 4;  // IV
    if (G < -0.00040) return 5;                   // V
    return -1;
}

// Класс усыхания по градиенту NDVI (на данным момент дублирует NDWI)
static int classifyByNdviGradient(double G, double R2) {
    return classifyByNdwiGradient(G, R2);
}

// Объединение классов NDVI и NDWI в итоговый класс.
// Логика:
// - если валиден только один индекс, используем его;
// - на ранних стадиях больший вес у NDWI;
// - на поздних стадиях больший вес у NDVI;
// - вес дополнительно масштабируется по качеству аппроксимации R².
static int combineGradientClasses(int cNdvi, double gNdvi, double r2Ndvi,
                                  int cNdwi, double gNdwi, double r2Ndwi) {
    Q_UNUSED(gNdvi)
    Q_UNUSED(gNdwi)

    const bool ndviOk = (cNdvi >= 0 && r2Ndvi >= 0.6);
    const bool ndwiOk = (cNdwi >= 0 && r2Ndwi >= 0.6);

    if (!ndviOk && !ndwiOk) return -1;
    if (ndviOk && !ndwiOk) return cNdvi;
    if (!ndviOk && ndwiOk) return cNdwi;

    // если оба показывают рост — рост
    if (cNdvi == 0 && cNdwi == 0) return 0;

    // если один рост, другой стабильность — считаем стабильностью
    if ((cNdvi == 0 && cNdwi == 1) || (cNdvi == 1 && cNdwi == 0)) return 1;

    // если один рост, а другой уже деградация — не даем росту "смазать"
    // проблему
    if (cNdvi == 0 && cNdwi >= 2) return cNdwi;
    if (cNdwi == 0 && cNdvi >= 2) return cNdvi;

    const int maxClass = qMax(cNdvi, cNdwi);

    double wNdwi = 0.5;
    double wNdvi = 0.5;

    // слабые классы: чуть больше доверяем NDWI
    if (maxClass <= 2) {
        wNdwi = 0.7;
        wNdvi = 0.3;
    }
    // средние: поровну
    else if (maxClass == 3) {
        wNdwi = 0.5;
        wNdvi = 0.5;
    }
    // тяжелые: чуть больше доверяем NDVI
    else {
        wNdwi = 0.3;
        wNdvi = 0.7;
    }

    wNdwi *= qMax(0.0, r2Ndwi - 0.6);
    wNdvi *= qMax(0.0, r2Ndvi - 0.6);

    if (wNdwi + wNdvi < 1e-12) return qMax(cNdvi, cNdwi);

    const double combined = (wNdwi * cNdwi + wNdvi * cNdvi) / (wNdwi + wNdvi);

    return qBound(0, qRound(combined), 5);
}

// Цвета классов по DP (аналог таблицы рис. 3.2)
static QColor dpClassColor(int dpClass) {
    switch (dpClass) {
        case 0:
            return QColor(0, 100, 120);  // Растет
        case 1:
            return QColor(34, 139, 34);  // Стабильно
        case 2:
            return QColor(120, 200, 80);  // Слабое ухудшение
        case 3:
            return QColor(255, 215, 0);  // Умеренное ухудшение
        case 4:
            return QColor(255, 140, 0);  // Сильное ухудшение
        case 5:
            return QColor(165, 42, 42);  // Очень сильное ухудшение
        default:
            return QColor(0, 0, 0, 0);
    }
}

MainWindowSatelliteComparator::MainWindowSatelliteComparator(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindowSatelliteComparator),
      m_scene(new QGraphicsScene),
      m_scene_cross_square_item(new CrossSquare(100)),
      m_dynamic_checkboxes_widget(nullptr),
      m_sat_comparator(new SatteliteComparator),
      m_is_image_created(false),
      m_is_bekas(false),
      m_scene_text_item_metric_value(new QGraphicsTextItem),
      bekas_window(nullptr),
      time_row_indexes_plot(new QCustomPlot_pro),
      m_speya_plot(new QCustomPlot_pro)

{
    ui->setupUi(this);
    m_lattitude = 0.0;
    m_longitude = 0.0;
    m_is_external_spectr = false;
    m_label_scene_coord = new QLabel;
    m_label_date_time = new QLabel;
    ui->statusbar->addPermanentWidget(m_label_scene_coord);
    ui->statusbar->addPermanentWidget(m_label_date_time);
    initUdpRpcConnection();
    setUpUi();
    QString app_title_version = "%1 %2 %3";
    setWindowTitle(app_title_version.arg(satc::app_name)
                       .arg(" ")
                       .arg(QString(VER_PRODUCTVERSION_STR)));
    gdal_start_driver();
    initSentinelStructs();
    initLandsatStructs();
    makeConnectsForMenuActions();
    addBaseItemsToScene();
    setUpPreviewPlot();
    connect(ui->graphicsView_satellite_image, SIGNAL(pointChanged(QPointF)),
            this, SLOT(cursorPointOnSceneChangedEvent(QPointF)));
    connect(ui->graphicsView_satellite_image, SIGNAL(sampleChanged(QPointF)),
            this, SLOT(samplePointOnSceneChangedEvent(QPointF)));
    ui->graphicsView_satellite_image->setUp(m_scene);
    setUpToolWidget();
    connect(ui->graphicsView_satellite_image,
            SIGNAL(roiPolygonAdded(const QString)), this,
            SLOT(add_roi_to_gui_list(const QString)));
    connect(ui->widget_image_saturation_light_corrector,
            SIGNAL(slidersWereChanged()), SLOT(updateImage()));

    connect(ui->widget_image_saturation_light_corrector,
            &SlidersOfImageCorrector::stretchParamsChanged, this,
            &MainWindowSatelliteComparator::onStretchParamsChanged);

    connect(ui->action_SpectraClassifer, SIGNAL(triggered()), this,
            SLOT(sendSpectrToMatlab()));
    // Shortcut Escape — заморозить/разморозить обновление графиков
    m_toggle_mouse_tracking_shortcut =
        new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(m_toggle_mouse_tracking_shortcut, &QShortcut::activated, this,
            &MainWindowSatelliteComparator::toggleMouseTracking);
    connect(&m_ac, SIGNAL(responseForCreatingImage()), this,
            SLOT(createImageWithAtmCorrecton()));  // action_sen2cor
    connect(&m_ac, SIGNAL(responseForLoadingSen2CorData()), this,
            SLOT(loadSentinelSen2Cor()));
    connect(&m_ac, SIGNAL(calculateStatisticSen2Cor_CATI()), this,
            SLOT(calculateSen2CorCATIaccuracy()));
}

MainWindowSatelliteComparator::~MainWindowSatelliteComparator() {
    delete ui;
    gdal_close_driver();
}

void MainWindowSatelliteComparator::toggleMouseTracking() {
    m_mouse_tracking_enabled = !m_mouse_tracking_enabled;
    qDebug() << m_mouse_tracking_enabled << "- m_mouse_tracking_enabled";
    // Визуальная индикация в статус-баре
    if (m_mouse_tracking_enabled) {
        ui->statusbar->showMessage(tr("Обновление графиков: ВКЛ"), 2000);
    } else {
        ui->statusbar->showMessage(
            tr("Обновление графиков: ОТКЛ (Esc для включения)"), 0);
    }
}

void MainWindowSatelliteComparator::openLandsat9HeaderData() {
    m_satelite_type = sad::LANDSAT_9;
    openCommonLandsatHeaderData(satc::satellite_name_landsat_9);
}

void MainWindowSatelliteComparator::openLandsat8HeaderData() {
    m_satelite_type = sad::LANDSAT_8;
    openCommonLandsatHeaderData(satc::satellite_name_landsat_8);
}

void MainWindowSatelliteComparator::openSentinel2AHeaderData() {
    m_satelite_type = sad::SENTINEL_2A;
    openCommonSentinelHeaderData(satc::satellite_name_sentinel_2A);
}

void MainWindowSatelliteComparator::openSentinel2BHeaderData() {
    m_satelite_type = sad::SENTINEL_2B;
    openCommonSentinelHeaderData(satc::satellite_name_sentinel_2B);
}

void MainWindowSatelliteComparator::openSentinel2CHeaderData() {
    m_satelite_type = sad::SENTINEL_2C;
    openCommonSentinelHeaderData(satc::satellite_name_sentinel_2C);
}

void MainWindowSatelliteComparator::openBekasSpectraData() {
    bekas_window = new UasvViewWindow;
    bekas_window->setWindowTitle(satc::app_name);
    bekas_window->setAttribute(Qt::WA_DeleteOnClose);
    connect(
        bekas_window,
        SIGNAL(
            sendSampleForSatelliteComparator(QVector<double>, QVector<double>)),
        this,
        SLOT(processBekasDataForComparing(QVector<double>, QVector<double>)));
    bekas_window->setUDPobj(m_rpc);
    bekas_window->show();
}

void MainWindowSatelliteComparator::openTimeRowData() {
    QString dir = QFileDialog::getExistingDirectory(
        this, "Выберите папку c временным рядом", QDir::homePath());
    QDir directory(dir);
    QStringList subdirs =
        directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug() << "before sorting by date time" << subdirs;
    if (subdirs.empty()) return;

    QStringList landsat_subdirs;
    QStringList sentinel_subdirs;

    for (const auto &subdir : qAsConst(subdirs)) {
        if (subdir.contains("S2B") || subdir.contains("S2A") ||
            subdir.contains("S2C")) {
            sentinel_subdirs.append(subdir);
        } else if (subdir.contains("LC09") || subdir.contains("LC08")) {
            landsat_subdirs.append(subdir);
        }
    };

    if (!landsat_subdirs.empty() && !sentinel_subdirs.empty()) {
        uts::showWarnigMessage(
            "Данные с разных спутников",
            "В папке могут быть изображения только для одного типа спутника.");
        return;
    }
    if (landsat_subdirs.empty() && sentinel_subdirs.empty()) {
        uts::showWarnigMessage("Нет данных", "Данные отсутсвуют");
        return;
    }
    deleteTimeRowData();
    QVector<sad::LANDSAT_METADATA_FILE> meta_datas;
    QVector<QString> date_time_row_stamps;

    if (!landsat_subdirs.empty()) {
        m_satelite_type = sad::TIME_ROW_LANDSAT_COMBINATION;
        subdirs = sortLandsatFilesByDateTime(landsat_subdirs);
        qDebug() << "landsat after sorting by date time" << subdirs;
        //!!!!! FILTER SUBDIRS

        m_time_row.resize(subdirs.size());
        m_time_row_geo.resize(m_time_row.size());
        m_time_row_qa_mask.resize(m_time_row.size());
        for (int i = 0; i < m_time_row.size(); ++i) {
            sad::LANDSAT_METADATA_FILE landsat_metadata;
            sad::geoTransform gt;
            m_time_row[i] = getDataFromJsonForLandsat8_9_TimeRow(
                directory.absolutePath() + "/" + subdirs[i] + "/" + subdirs[i] +
                    "_MTL.json",
                landsat_metadata, gt);
            sad::QA_MASK_DATA qa_mask;
            qa_mask.file_name = directory.absolutePath() + "/" + subdirs[i] +
                                "/" + subdirs[i] + "_QA_PIXEL.TIF";
            qa_mask.data =
                readTiff(qa_mask.file_name, qa_mask.width, qa_mask.height);
            //            qDebug() << "mask_widht -- mask_height: " <<
            //            qa_mask.width
            //                     << qa_mask.height << qa_mask.file_name;
            m_time_row_qa_mask[i] = qa_mask;

            meta_datas.push_back(landsat_metadata);
            date_time_row_stamps.push_back(
                landsat_metadata.image_attributes.date_acquired);
            m_time_row_geo[i] = gt;
        }

    } else {
        m_satelite_type = sad::TIME_ROW_SENTINEL_COMBINATION;
        subdirs = sortSentinelFilesByDateTime(sentinel_subdirs);
        qDebug() << "sentinel time row after sorting by date time" << subdirs;

        // WORK IN PROGRESS (WIP)
        m_time_row.resize(subdirs.size());
        m_time_row_geo.resize(m_time_row.size());
        m_time_row_qa_mask.resize(m_time_row.size());

        for (int i = 0; i < m_time_row.size(); ++i) {
            sad::SENTINEL_METADATA sentinel_metadata;
            sad::geoTransform gt;
            m_time_row[i] = getDataForSentinel_TimeRow(
                directory.absolutePath() + "/" + subdirs[i] + "/MTD_MSIL2A.xml",
                sad::SENTINEL_2A, sentinel_metadata, gt);

            /*sad::QA_MASK_DATA qa_mask;
            qa_mask.file_name = directory.absolutePath() + "/" + subdirs[i]+"/"
            +  subdirs[i] + "_QA_PIXEL.TIF"; qa_mask.data =
            readTiff(qa_mask.file_name,qa_mask.width,qa_mask.height);
            //qDebug()<<"mask_widht -- mask_height:
            "<<qa_mask.width<<qa_mask.height; m_time_row_qa_mask[i] = qa_mask;*/

            sad::QA_MASK_DATA qa_mask;
            const QString sceneRoot =
                directory.absolutePath() + "/" + subdirs[i];

            qa_mask.file_name = sceneRoot;
            qa_mask.data =
                loadMaskForSentinel(qa_mask.width, qa_mask.height, sceneRoot);

            qa_mask.scl_file_name = sceneRoot;
            qa_mask.scl_data = loadSCLForSentinel(
                qa_mask.scl_width, qa_mask.scl_height, sceneRoot);

            qDebug() << "cloud mask width -- height:" << qa_mask.width
                     << qa_mask.height;
            qDebug() << "scl mask width -- height:" << qa_mask.scl_width
                     << qa_mask.scl_height;

            m_time_row_qa_mask[i] = qa_mask;

            date_time_row_stamps.push_back(
                sentinel_metadata.image_attributes.date_acquired);
            qDebug() << "Date time row stamp: "
                     << sentinel_metadata.image_attributes.date_acquired;
            qDebug() << QString(
                            "######## load image^. Number of its bands = %1")
                            .arg(m_time_row[i].size());
            m_time_row_geo[i] = gt;
        }
    }

    base_pixel_geo = new PixelGeo(m_time_row_geo[0]);

    // Будем добавлять график, если его не хватает для новых временных точек
    while (m_time_row.size() > m_preview_plot->graphCount()) {
        m_preview_plot->addGraph();
    };  //
    for (int i = 0; i < m_preview_plot->graphCount(); i++) {
        if (i < distinctColors.size()) {
            distinctColors.append(randomNiceColor());
        }
        m_preview_plot->graph(i)->setPen(QColor(distinctColors[i]));
    }
    m_preview_plot->legend->setVisible(false);
    QStringList gui_available_bands;
    for (int j = 0; j < m_time_row[0].size(); ++j) {
        gui_available_bands << m_time_row[0][j].gui_name;
    };
    if (m_dynamic_checkboxes_widget) {
        m_dynamic_checkboxes_widget->clear();
        delete m_dynamic_checkboxes_widget;
    }
    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(
        gui_available_bands, ui->verticalLayout_satellite_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1, 2, 3});
    ui->statusbar->showMessage("");
    m_is_image_created = true;
    m_scene_cross_square_item->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);
    change_bands_and_show_image(m_time_row[0]);  // NEED REFACTORING

    syncManager = new ViewSyncManager;

    auto imgs = get_cropedImages_for_time_row(m_time_row, m_satelite_type);

    for (int i = 0; i < imgs.size(); ++i) {
        ImageViewer *viewer = new ImageViewer;
        m_viewers.push_back(viewer);
        QPixmap pixmap = QPixmap::fromImage(imgs[i]);
        viewer->setImage(pixmap);
        viewer->resize(400, 400);
        viewer->connectSync(syncManager);

        QString date = date_time_row_stamps[i];
        int r = distinctColors[i].red();
        int g = distinctColors[i].green();
        int b = distinctColors[i].blue();
        QIcon icon = iut::createIcon(r, g, b, QSize(25, 25));

        QWidget *container = new QWidget(viewer);

        QHBoxLayout *layout = new QHBoxLayout(container);

        // Иконка
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(icon.pixmap(25, 25));
        layout->addWidget(iconLabel);

        // Текст
        QLabel *textLabel = new QLabel(date);
        QFont font("Arial", 12);
        textLabel->setFont(font);
        textLabel->setStyleSheet("color: white;");
        layout->addWidget(textLabel);

        // Стиль контейнера: полупрозрачный тёмный фон
        container->setStyleSheet(
            "background-color: rgba(0, 0, 0, 150); border-radius: 3px;");
        container->setLayout(layout);
    }

    QGridLayout *layout = new QGridLayout;
    int row = 0, col = 0;
    for (int i = 0; i < m_viewers.size(); ++i) {
        layout->addWidget(m_viewers[i], row, col);
        col++;
        if (col == 3) {
            col = 0;
            row++;
        }
    }
    m_time_row_widget.setLayout(layout);
    m_time_row_widget.setWindowTitle("Временной ряд");

    m_time_row_widget.show();
    // paintTimeRowBadForest(Qt::black);
}

void MainWindowSatelliteComparator::findAreasUsingSelectedMetric() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
    QString message =
        QString("Пожалуйста подождите,\nпроисходит поиск областей\n(%1)...")
            .arg(m_comboBox_calculation_method->currentText());
    ProgressInformator progress_info(ui->graphicsView_satellite_image, message);
    progress_info.show();
    QApplication::processEvents();
    paintSamplePoints(color);
    progress_info.close();
}

void MainWindowSatelliteComparator::centerSceneOnCrossSquare() {
    ui->graphicsView_satellite_image->centerOn(m_scene_cross_square_item);
    ui->graphicsView_satellite_image->setTransform(QTransform());
}

void MainWindowSatelliteComparator::cursorPointOnSceneChangedEvent(
    QPointF pos) {
    if (!m_mouse_tracking_enabled) return;

    if (m_satelite_type == sad::TIME_ROW_LANDSAT_COMBINATION) {
        cursorPointOnSceneChangedEventTimeRow(pos, true);
        return;
    } else if (m_satelite_type == sad::TIME_ROW_SENTINEL_COMBINATION) {
        cursorPointOnSceneChangedEventTimeRow(pos, false);
        return;
    }
    QVector<double> data;
    QVector<double> waves;
    QVector<double> sample;
    QVector<double> trimmed_satellite_data;

    const QString x_y = "x: %1   y:%2";
    QString x_y_message =
        x_y.arg(QString::number(pos.x()), QString::number(pos.y()));
    m_label_scene_coord->setText(x_y_message);

    if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
        m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
        data = getLandsat8Ksy(pos.x(), pos.y());
    } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2C) {
        auto w_k = getSentinelKsy(pos.x(), pos.y());
        data = w_k.second;
        waves = w_k.first;
        if (m_sentinel_sample.empty()) {
            sample = data;  // TEMPORARY
        } else {
            sample = m_sentinel_sample;
        }
        trimmed_satellite_data = data;
        auto bv = getBandsValues(waves, data, m_satelite_type);

        // clang-format off
       /* qDebug() << "blue_cw" << blue_cw  <<blue_value;
        qDebug() << "red_cw"  << red_cw   <<red_value;
        qDebug() << "nir1_cw" << nir1_cw  <<nir1_value;
        qDebug() << "swir2" << swir3_cw <<swir3_value;
        qDebug() << "----------------------";*/
        // clang-format on

        double dswi = sam::calculateDSWI(bv.nir1, bv.green, bv.swir2, bv.red);
        double evi = sam::calculateEVI(bv.nir1, bv.red, bv.blue);
        double ndvi = sam::calculateNDVI(bv.nir1, bv.red);
        double swvi = sam::calculateSWVI(bv.nir1, bv.swir2);

        m_spectralWidget->setIndices({{sam::kSpectralIndexNDVI, ndvi},
                                      {{sam::kSpectralIndexSWVI}, {swvi}},
                                      {{sam::kSpectralIndexDSWI}, {dswi}},
                                      {sam::kSpectralIndexEVI, evi}});
    }
    if (data.empty()) {
        return;
    }

    if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
        m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
        if (data.size() != (int)LANDSAT_BANDS_NUMBER - 4) {
            qDebug() << "ERROR SIZE:" << data.size();
            return;
        }
        if (m_is_bekas) {
            sample = m_bekas_sample;
            waves = waves_landsat9_5;
            size_t elems_to_copy = std::min(static_cast<size_t>(data.size()),
                                            static_cast<size_t>(5));
            trimmed_satellite_data =
                data.mid(0, static_cast<int>(elems_to_copy));
        } else {
            sample = m_landsat9_sample;
            waves = waves_landsat9;
            trimmed_satellite_data = data;
        }
    } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2C) {
        if (m_is_bekas) {
            sample = m_bekas_sample;
            waves = waves_sentinel_2c_5;
            size_t elems_to_copy = std::min(static_cast<size_t>(data.size()),
                                            static_cast<size_t>(9));
            trimmed_satellite_data =
                data.mid(0, static_cast<int>(elems_to_copy));
        } else {
            QVector<double> c_waves;
            for (int i = 0; i < m_sentinel_data.size(); ++i) {
                c_waves.push_back(m_sentinel_data[i].central_wave_length);
            }
            sample = m_sentinel_sample;
            waves = c_waves;
            trimmed_satellite_data = data;
        }
    }

    m_preview_plot->graph(0)->data().clear();
    m_preview_plot->graph(1)->data().clear();
    if (waves.size() == trimmed_satellite_data.size()) {
        m_preview_plot->graph(0)->setData(waves, trimmed_satellite_data);
    }
    if (waves.size() == sample.size()) {
        m_preview_plot->graph(1)->setData(waves, sample);
    }

    double result = 999;
    if (m_comboBox_calculation_method->currentText() == satc::spectral_angle) {
        result = calculateSpectralAngle(trimmed_satellite_data, sample);
    } else if (m_comboBox_calculation_method->currentText() ==
               satc::euclid_metrika) {
        result = euclideanDistance(trimmed_satellite_data, sample);
    }
    m_scene_text_item_metric_value->setPos(pos.x(), pos.y() + 5);
    m_scene_text_item_metric_value->setPlainText(QString::number(result));
    m_preview_plot->rescaleAxes(true);
    m_preview_plot->replot();

    double lat, lon;
    auto geo_coord_str =
        getGeoCoordinates(pos.x(), pos.y(), m_geo, lat, lon, true);
    ui->statusbar->showMessage(geo_coord_str);
    auto speya_data = getSentinelSpeyaValues(pos.x(), pos.y());
    auto sen2cor_ksy = getSen2CorKsy(pos.x(), pos.y());
    if (!m_sen2cor_data.empty()) {
        auto class_value =
            mask_for_sen2cor_data[(int)pos.y() * m_sen2cor_data[0].width +
                                  (int)pos.x()];
        auto class_name = getSclClassName(class_value);
        qDebug() << "sentinel class: " << class_name;
        m_ac.showSentinelClassName(class_name);
    }
    m_ac.showAlbedoUnderCursor(speya_data, sen2cor_ksy);
    m_speya_plot->graph(0)->setData(waves, speya_data);
    m_speya_plot->rescaleAxes(true);
    m_speya_plot->replot();
}

void MainWindowSatelliteComparator::samplePointOnSceneChangedEvent(
    QPointF pos) {
    m_is_bekas = false;
    m_scene_cross_square_item->setPos(pos);
    m_scene_cross_square_item->update();
    double lat = 0.0;
    double longitude = 0.0;

    if (m_satelite_type == sad::TIME_ROW_LANDSAT_COMBINATION ||
        m_satelite_type == sad::TIME_ROW_SENTINEL_COMBINATION) {
        getGeoCoordinates(pos.x(), pos.y(), m_time_row_geo[0], lat, longitude,
                          false);
        m_lattitude = lat;
        m_longitude = longitude;
        return;
    }
    getGeoCoordinates(pos.x(), pos.y(), m_geo, lat, longitude, false);
    m_lattitude = lat;
    m_longitude = longitude;

    QVector<double> data;
    QVector<double> sample;
    QVector<double> waves;
    if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9 ||
        m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8) {
        data = getLandsat8Ksy(pos.x(), pos.y());

        if (data.empty()) return;
        if (data.size() != (int)LANDSAT_BANDS_NUMBER - 4) {
            qDebug() << "ERROR SIZE:" << data.size();
            return;
        }

        m_landsat9_sample = data;
        sample = m_landsat9_sample;
        waves = waves_landsat9;

    } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2C) {
        auto w_k = getSentinelKsy(pos.x(), pos.y());
        data = w_k.second;
        waves = w_k.first;
        m_sentinel_sample = data;
        sample = m_sentinel_sample;
    }
    auto speya_values = getSentinelSpeyaValues(pos.x(), pos.y());
    copyVectorsToClipboard(m_lattitude, m_longitude, waves,
                           sample);  // speya_values);
    m_ac.updateBasePixel(speya_values);
    m_preview_plot->graph(0)->data().clear();
    m_preview_plot->graph(1)->data().clear();
    m_preview_plot->graph(0)->setData(waves, data);
    m_preview_plot->graph(1)->setData(waves, sample);
    m_preview_plot->rescaleAxes(true);
    m_preview_plot->replot();
}

void MainWindowSatelliteComparator::openCommonLandsatHeaderData(
    const QString &satellite_name) {
    QString openSatMessage =
        QString("Открыть заголовочный файл %1").arg(satellite_name);
    QString headerName = QFileDialog::getOpenFileName(
        this, openSatMessage, "", "файлы(*_MTL.json *_MTL.txt *_MTL.xml)");
    ui->graphicsView_satellite_image->setIsSignal(false);
    clearLandsat9DataBands();
    clear_satellite_data();
    clear_all_layers();
    deleteTimeRowData();
    m_scene_cross_square_item->setVisible(false);
    QFile file(headerName);
    static bool isHeaderValid = false;
    if (file.exists() == false) return;
    if (m_dynamic_checkboxes_widget) m_dynamic_checkboxes_widget->clear();
    QFileInfo fi(headerName);
    m_root_path = fi.path();
    const QString extension = fi.completeSuffix();
    QString dataLoadingMessage =
        QString("Загрузка данных %1...").arg(satellite_name);
    ui->statusbar->showMessage(dataLoadingMessage);
    QApplication::processEvents();
    QList<QString> landsat_gui_available_bands;

    if (extension == "json") {
        QJsonObject jo;
        jsn::getJsonObjectFromFile(headerName, jo);
        if (jo.contains("LANDSAT_METADATA_FILE")) {
            QJsonObject image_attributes =
                jsn::getValueByPath(
                    jo, {"LANDSAT_METADATA_FILE", "IMAGE_ATTRIBUTES"})
                    .toObject();
            title_satellite_name->setText(
                image_attributes.value("SPACECRAFT_ID").toString());
            qDebug() << image_attributes.value("DATE_ACQUIRED").toString();
            qDebug() << image_attributes.value("SCENE_CENTER_TIME").toString();
            QJsonValue value = jsn::getValueByPath(
                jo, {"LANDSAT_METADATA_FILE", "PRODUCT_CONTENTS"});
            QJsonValue radiance_value = jsn::getValueByPath(
                jo, {"LANDSAT_METADATA_FILE",
                     "LEVEL2_SURFACE_REFLECTANCE_PARAMETERS"});
            QJsonObject check_bands = value.toObject();
            QJsonObject radiance = radiance_value.toObject();
            QJsonObject projection =
                jsn::getValueByPath(
                    jo, {"LANDSAT_METADATA_FILE", "PROJECTION_ATTRIBUTES"})
                    .toObject();
            m_geo.utmZone = projection["UTM_ZONE"].toString().toDouble();
            m_geo.ulX = projection["CORNER_UL_PROJECTION_X_PRODUCT"]
                            .toString()
                            .toDouble();
            m_geo.ulY = projection["CORNER_UL_PROJECTION_Y_PRODUCT"]
                            .toString()
                            .toDouble();
            m_geo.resX =
                projection["GRID_CELL_SIZE_REFLECTIVE"].toString().toDouble();
            m_geo.resY = -m_geo.resX;

            for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
                if (check_bands.value(sad::landsat9_bands_keys[i])
                        .isUndefined()) {
                    qDebug() << "missed band: " << sad::landsat9_bands_keys[i];
                    m_landsat9_missed_channels[i] = true;
                    continue;
                }
                m_landsat9_missed_channels[i] = false;
                landsat_gui_available_bands.append(
                    sad::landsat_bands_gui_names[i]);
                auto band_file_name =
                    check_bands[sad::landsat9_bands_keys[i]].toString();
                int xS;
                int yS;
                m_landsat9_data_bands[i] =
                    readTiff(fi.path() + "/" + band_file_name, xS, yS);
                m_landsat9_bands_image_sizes[i] = {xS, yS};

                if (i < 7) {  // TODO TEST Есть только до 9 канала TEST
                              // REFLECTANCE_MULT_BAND_9  REFLECTANCE_ADD_BAND_9
                    double mult_refl =
                        radiance[sad::landsat9_mult_reflectence_keys[i]]
                            .toString()
                            .toDouble();
                    double add_refl =
                        radiance[sad::landsat9_add_reflectence_keys[i]]
                            .toString()
                            .toDouble();
                    m_reflectance_mult_add_arrays[i][0] = mult_refl;
                    m_reflectance_mult_add_arrays[i][1] = add_refl;
                }
            }
            isHeaderValid = true;
        } else {
            return;  // Пока работает только LANDSAT 9
        };
    } else if (extension == "txt") {
        auto file_names = getLandSat9BandsFromTxtFormat(
            headerName, landsat_gui_available_bands);
        // qDebug()<<"TXT filenames: "<<file_names;
        title_satellite_name->setText(
            getLandSatSpaceCraftIDFromTxtFormat(headerName));
        read_landsat_bands_data(file_names);
        fillLandSat9ReflectanceMultAdd(headerName);
        fillLandSat9GeoData(headerName);
        isHeaderValid = true;
    } else if (extension == "xml") {
        auto data = satc::readLandsatXmlHeader(headerName);
        title_satellite_name->setText(data.image_attributes.spacecraft_id);
        QStringList file_names;
        for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
            if (data.landsat9_missed_channels[i]) continue;
            file_names.append(data.product_contents.file_name_bands[i]);
            landsat_gui_available_bands.append(sad::landsat_bands_gui_names[i]);
            m_reflectance_mult_add_arrays[i][0] =
                data.level2_surface_reflectance_parameters
                    .reflectance_mult_band[i]
                    .toDouble();
            m_reflectance_mult_add_arrays[i][1] =
                data.level2_surface_reflectance_parameters
                    .reflectance_add_band[i]
                    .toDouble();
        }
        read_landsat_bands_data(file_names);
        m_geo.utmZone = data.projection_attributes.utm_zone.toDouble();
        m_geo.resX =
            data.projection_attributes.grid_cell_size_reflective.toDouble();
        m_geo.resY = -m_geo.resX;
        m_geo.ulX = data.projection_attributes.corner_ul_projection_x_product
                        .toDouble();
        m_geo.ulY = data.projection_attributes.corner_ul_projection_y_product
                        .toDouble();
        if (!data.isHeaderValid) return;
        isHeaderValid = true;
    }
    if (isHeaderValid == false) {
        m_satelite_type = sad::UNKNOWN_SATELLITE;
        return;
    }

    // fill universal struct instead of specific (WORK IN PROGRESS)
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        if (m_landsat9_missed_channels
                [sad::sorted_landsat_bands_order_by_wavelength[i]])
            continue;
        qDebug() << "----check test---"
                 << sad::landsat_bands_gui_names
                        [sad::sorted_landsat_bands_order_by_wavelength[i]];
    }

    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(
        landsat_gui_available_bands, ui->verticalLayout_satellite_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1, 2, 3});
    connect(m_dynamic_checkboxes_widget, SIGNAL(choosed_bands_changed()), this,
            SLOT(change_bands_and_show_image()));

    change_bands_and_show_image();
    ui->statusbar->showMessage("");
    m_is_image_created = true;
    m_scene_cross_square_item->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);
}

void MainWindowSatelliteComparator::openCommonSentinelHeaderData(
    const QString &satellite_name) {
    QString headerName = getPathToSentinelHeader(this, satellite_name);

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
    deleteTimeRowData();
    m_sentinel_metadata.solar_irradiance =
        satc::extractSolarIrradianceForSentinel(headerName);
    QFileInfo fi(headerName);
    m_root_path = fi.path();
    m_sentinel_metadata.root_path = fi.path();
    QString dataLoadingMessage =
        QString("Загрузка данных %1...").arg(satellite_name);
    ui->statusbar->showMessage(dataLoadingMessage);
    QApplication::processEvents();

    QStringList imageFiles;
    QDomNodeList imageNodes = doc.elementsByTagName("IMAGE_FILE");
    for (int i = 0; i < imageNodes.count(); ++i) {
        QDomNode node = imageNodes.at(i);
        imageFiles << node.toElement().text();
    }
    QStringList filteredFiles;

    for (const QString &file : qAsConst(imageFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            // Ищем точное вхождение ключа как часть имени файла
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                filteredFiles << file;
                break;  // нашли — переходим к следующему файлу
            }
        }
    }
    // qDebug() << "check files: " << filteredFiles;
    QStringList finalFiles;
    QMap<QString, QString> bestResolutionForBand;
    const QStringList priorityOrder = {"R20m", "R10m", "R60m"};

    // Для каждого band ищем путь с наивысшим приоритетом по разрешению
    for (const QString &bandKey : sad::sentinel_bands_keys) {
        for (const QString &resolution : priorityOrder) {
            for (const QString &file : qAsConst(filteredFiles)) {
                if (file.contains(resolution) &&
                    file.contains("_" + bandKey + "_")) {
                    bestResolutionForBand[bandKey] = file;
                    break;  // нашли лучший — переходим к следующему band
                }
            }
            if (bestResolutionForBand.contains(bandKey))
                break;  // если уже найден — не ищем в меньших разрешениях
        }
    }

    // Собираем финальный список
    finalFiles = bestResolutionForBand.values();
    qDebug() << "final files: " << finalFiles;
    title_satellite_name->setText(satellite_name);

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        QString target = "_" + sad::sentinel_bands_keys[i] + "_";
        QStringList list = finalFiles.filter(target);
        if (list.size() == 1) {
            m_sentinel_metadata.sentinel_missed_channels[i] = false;
            m_sentinel_metadata.files[i] = list.at(0);
        } else if (list.size() > 1) {
            qDebug() << "DUBLICATED FILES IN FINAL FILES LIST...";
        }
        target = "";
    }

    if (m_dynamic_checkboxes_widget) m_dynamic_checkboxes_widget->clear();
    QList<QString> availableBandNames;
    QString gui_channels[SENTINEL_BANDS_NUMBER];
    double central_waves[SENTINEL_BANDS_NUMBER];
    if (m_satelite_type == sad::SENTINEL_2A) {
        copyQStringArray(sad::sentinel_2A_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2A_central_wave_lengths,
                  sad::sentinel_2A_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (m_satelite_type == sad::SENTINEL_2B) {
        copyQStringArray(sad::sentinel_2B_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2B_central_wave_lengths,
                  sad::sentinel_2B_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (m_satelite_type == sad::SENTINEL_2C) {
        copyQStringArray(sad::sentinel_2C_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2C_central_wave_lengths,
                  sad::sentinel_2C_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    }

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        if (!m_sentinel_metadata.sentinel_missed_channels[i]) {
            sad::BAND_DATA data;
            // if (gui_channels[i].contains("WV")) continue;
            availableBandNames << gui_channels[i];
            data.gui_name = gui_channels[i];
            data.solar_irradiance = m_sentinel_metadata.solar_irradiance[i];
            data.central_wave_length = central_waves[i];
            data.file_name = m_sentinel_metadata.files[i];

            bool isResolutionMissed = true;
            for (const QString &resolution : priorityOrder) {
                if (data.file_name.contains(resolution)) {
                    data.resolution_in_pixel_meters = resolution;
                    data.width =
                        sad::sentinel_resolutions.value(resolution).first;
                    data.height =
                        sad::sentinel_resolutions.value(resolution).second;
                    isResolutionMissed = false;
                    qDebug() << "r, w, h: " << data.resolution_in_pixel_meters
                             << data.width << data.height;
                    break;
                };
            }
            if (isResolutionMissed) {
                // TODO EXCEPTION
                // Мы обязательно должны знать разрешение
                //  Выбросить исключение
                qDebug() << "<--------------------- NO RESOLUTION EXCEPTION "
                            "!!!------------------>";
            }
            m_sentinel_data.append(data);
        }
    }
    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(
        availableBandNames, ui->verticalLayout_satellite_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1, 2, 3});

    connect(m_dynamic_checkboxes_widget, SIGNAL(choosed_bands_changed()), this,
            SLOT(change_bands()));

    read_sentinel2_bands_data(m_sentinel_data, m_root_path);
    change_bands_and_show_image(m_sentinel_data);

    ui->statusbar->showMessage("");
    m_is_image_created = true;
    m_scene_cross_square_item->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);
    QHash<QString, sad::geoTransform> sentinel_geo;
    if (finalFiles.empty() == false) {
        QFileInfo finfo(m_root_path + "/" + finalFiles[0] + ".jp2");
        QDir dir(finfo.absolutePath());
        dir.cdUp();
        dir.cdUp();
        const QString geo_file = dir.path() + "/MTD_TL.xml";
        fi.setFile(geo_file);
        auto xml_doc = fi.absoluteFilePath();
        qDebug() << xml_doc << "--->" << fi.exists();
        m_geo.utmZone = extractUTMZoneFromXML(xml_doc, m_geo);
        sentinel_geo = extractGeoPositions(xml_doc);
        m_geo.ulX = sentinel_geo["20"].ulX;
        m_geo.ulY = sentinel_geo["20"].ulY;
        m_geo.resX = 20;
        m_geo.resY = -20;

        QString date_time =
            getDateTimeFromXML(xml_doc).toString("yyyy/MM/dd hh:mm:ss");

        m_sentinel_metadata.image_attributes.date_acquired = date_time;
        m_label_date_time->setText(date_time);
        // TODO CHECK AND WARN USER ABOUT WRONG VALUES AND ERRORS
        double sunZenitAngle = satc::getSunZenitAngleForSentinel(xml_doc);
        double sunAzimutAngle = satc::getSunAzimuthAngleForSentinel(xml_doc);
        m_sentinel_metadata.sunZenithAngle = sunZenitAngle;
        m_sentinel_metadata.sunAzimuthAngle = sunAzimutAngle;
        m_sentinel_metadata.cosSunZenithAngle =
            cos(M_PI / 180.0 * sunZenitAngle);
        qDebug() << "sza" << sunZenitAngle << "saa" << sunAzimutAngle
                 << "cosSun" << m_sentinel_metadata.cosSunZenithAngle;
    }
}

void MainWindowSatelliteComparator::processBekasDataForComparing(
    const QVector<double> &x, const QVector<double> &y) {
    // qDebug()<<"sat_comparator: "<<x.size()<<y.size();
    if (m_satelite_type == sad::UNKNOWN_SATELLITE) return;

    if (m_satelite_type == sad::LANDSAT_9) {
        m_sat_comparator->set_satellite_responses("landsat9");
        m_sat_comparator->initial_fill_data_to_show(x, y, waves_landsat9,
                                                    m_landsat9_sample);
    } else if (m_satelite_type == sad::LANDSAT_8) {
        m_sat_comparator->set_satellite_responses("landsat8");
        m_sat_comparator->initial_fill_data_to_show(x, y, waves_landsat9,
                                                    m_landsat9_sample);
    } else if (m_satelite_type == sad::SENTINEL_2A) {
        m_sat_comparator->set_satellite_responses("sentinel2A");  // sentinel2C
        m_sat_comparator->initial_fill_data_to_show(x, y,
                                                    waves_sentinel_2c,  // TEMP
                                                    m_sentinel_sample);
    } else if (m_satelite_type == sad::SENTINEL_2B) {
        m_sat_comparator->set_satellite_responses("sentinel2B");  // sentinel2C
        m_sat_comparator->initial_fill_data_to_show(x, y,
                                                    waves_sentinel_2c,  // TEMP
                                                    m_sentinel_sample);
    } else if (m_satelite_type == sad::SENTINEL_2C) {
        m_sat_comparator->set_satellite_responses("sentinel2C");  // sentinel2C
        m_sat_comparator->initial_fill_data_to_show(x, y, waves_sentinel_2c,
                                                    m_sentinel_sample);
        // qDebug() << "x: " << x;
        // qDebug() << "y: " << y;
        qDebug() << "waves_sentinel_2c: " << waves_sentinel_2c;
        qDebug() << "sentinel_sample: " << m_sentinel_sample;
    }
    auto folded_device_spectr =
        m_sat_comparator->fold_spectr_to_satellite_responses();
    qDebug() << "folded_device_spectr: " << folded_device_spectr.size();
    if (folded_device_spectr.empty()) {
        m_is_bekas = false;
        return;
    }
    m_bekas_sample = folded_device_spectr;
    m_is_bekas = true;
}

void MainWindowSatelliteComparator::handleJsonRpcResult(
    const QJsonValue &result) {
    qDebug() << "call handleJsonRpcResult from MainWindowSatelliteComparator";
    qDebug() << "result:" << result.toString();
}

void MainWindowSatelliteComparator::processTestMatlabRequest(
    const QVariantMap &params) {
    QString path = params["path"].toString();
    int mode = params["mode"].toInt();
    qDebug() << "matlab вызывал Test и передал параметры: " << path << " "
             << mode;
}

void MainWindowSatelliteComparator::processpClassifiedBecasSpectraMatlabRequest(
    const QVariantMap &params) {
    QString pathMatfile = params["matFilePath"].toString();
    MatFilesOperator reader;
    BecasDataFromMatlab dataReaded =
        reader.readBecasDataFromMatlab(pathMatfile);
    if (dataReaded.isSomeErrors) {
        qDebug() << "при чтении файла произошли ошибки";
        return;
    }

    bekas_window->updateListWithClustNums(dataReaded.specNames,
                                          dataReaded.selectedClustIndxs,
                                          dataReaded.colorsOfEachSpectr);
}

void MainWindowSatelliteComparator::processpClassifiedMultiSpecMatlabRequest(
    const QVariantMap &params) {
    qDebug() << "зашли в processpClassifiedMultiSpecMatlabRequest";
    QString pathMatfile = params["matFilePath"].toString();
    MatFilesOperator reader;
    MultiSpecDataFromMatlab dataReaded =
        reader.readMultiSpecDataFromMatlab(pathMatfile);
    if (dataReaded.isSomeErrors) {
        qDebug() << "при чтении файла произошли ошибки";
        return;
    }
    paintMultiSpecPoints(dataReaded.pixelX, dataReaded.pixelY,
                         dataReaded.colorsOfEachSpectr);
}

void MainWindowSatelliteComparator::updateImage() {
    if (m_image_item) {
        double coef_saturation =
            ui->widget_image_saturation_light_corrector->getCoefSaturation();
        double coef_light =
            ui->widget_image_saturation_light_corrector->getCoefLight();
        QImage imgNew =
            createModifiedImage(m_satellite_image, coef_saturation, coef_light);
        auto pixmap = QPixmap::fromImage(imgNew);
        m_scene->removeItem(m_image_item);
        delete m_image_item;
        m_image_item = new QGraphicsPixmapItem(pixmap);
        m_image_item->setZValue(0);
        m_image_item->setCursor(Qt::CrossCursor);
        m_scene->addItem(m_image_item);
        m_scene->update();
    }
}

void MainWindowSatelliteComparator::runChangeDetectionMethod(
    const QString &polygonId) {
    QString openSatMessage =
        QString("Открыть заголовочный файл %1").arg("Sentinel");
    QString headerName = getPathToSentinelHeader(this, openSatMessage);

    // DUBLICATED CODE CHANGE DETECTION FROM COMMON SENTINEL

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
    int wCloudM, hCloudM;

    dataCloudMask =
        loadMaskForSentinel(wCloudM, hCloudM, m_sentinel_metadata.root_path);

    QFileInfo fi(headerName);
    m_root_path = fi.path();
    int wCloudM2, hCloudM2;
    dataCloudMask2 = loadMaskForSentinel(wCloudM2, hCloudM2, fi.path());

    QStringList imageFiles;
    QDomNodeList imageNodes = doc.elementsByTagName("IMAGE_FILE");
    for (int i = 0; i < imageNodes.count(); ++i) {
        QDomNode node = imageNodes.at(i);
        imageFiles << node.toElement().text();
    }
    QStringList filteredFiles;

    for (const QString &file : qAsConst(imageFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            // Ищем точное вхождение ключа как часть имени файла
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                filteredFiles << file;
                break;  // нашли — переходим к следующему файлу
            }
        }
    }
    QStringList finalFiles;
    QMap<QString, QString> bestResolutionForBand;
    const QStringList priorityOrder = {"R20m", "R10m", "R60m"};

    for (const QString &bandKey : sad::sentinel_bands_keys) {
        for (const QString &resolution : priorityOrder) {
            for (const QString &file : qAsConst(filteredFiles)) {
                if (file.contains(resolution) &&
                    file.contains("_" + bandKey + "_")) {
                    bestResolutionForBand[bandKey] = file;
                    break;
                }
            }
            if (bestResolutionForBand.contains(bandKey)) break;
        }
    }

    finalFiles = bestResolutionForBand.values();

    sad::SENTINEL_METADATA temp_metadata;
    QFutureWatcher<QPixmap> *change_detection_future =
        new QFutureWatcher<QPixmap>();
    QVector<sad::BAND_DATA> change_detection_data;
    sad::geoTransform change_detection_geo;

    for (const QString &file : qAsConst(finalFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                temp_metadata.sentinel_missed_channels[i] = false;
                temp_metadata.files[i] = file;
                qDebug() << i << "----CD------->" << file;
                break;
            }
        }
    }

    QList<QString> availableBandNames;
    QString gui_channels[SENTINEL_BANDS_NUMBER];
    double central_waves[SENTINEL_BANDS_NUMBER];
    if (m_satelite_type == sad::SENTINEL_2A) {
        copyQStringArray(sad::sentinel_2A_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2A_central_wave_lengths,
                  sad::sentinel_2A_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (m_satelite_type == sad::SENTINEL_2B) {
        copyQStringArray(sad::sentinel_2B_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2B_central_wave_lengths,
                  sad::sentinel_2B_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    }

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        if (!temp_metadata.sentinel_missed_channels[i] &&
            (i == sam::_NIR2 || i == sam::_SWIR2 || i == sam::_SWIR3)) {
            availableBandNames << gui_channels[i];
            sad::BAND_DATA data;
            data.gui_name = gui_channels[i];
            data.central_wave_length = central_waves[i];
            data.file_name = temp_metadata.files[i];

            bool isResolutionMissed = true;
            for (const QString &resolution : priorityOrder) {
                if (data.file_name.contains(resolution)) {
                    data.resolution_in_pixel_meters = resolution;
                    data.width =
                        sad::sentinel_resolutions.value(resolution).first;
                    data.height =
                        sad::sentinel_resolutions.value(resolution).second;
                    isResolutionMissed = false;
                    qDebug() << "r, w, h: " << data.resolution_in_pixel_meters
                             << data.width << data.height;
                    break;
                };
            }
            if (isResolutionMissed) {
                // TODO EXCEPTION
                // Мы обязательно должны знать разрешение
                //  Выбросить исключение
                qDebug() << "<--------------------- NO RESOLUTION EXCEPTION "
                            "!!!------------------>";
            }
            change_detection_data.append(data);
        } else {
            temp_metadata.sentinel_missed_channels[i] = false;
        }
    }
    int cd_size = change_detection_data.size();
    qDebug() << "change detection size: " << cd_size;
    if (cd_size == 3) {
        qDebug() << change_detection_data[0].gui_name
                 << change_detection_data[0].file_name;
        qDebug() << change_detection_data[1].gui_name
                 << change_detection_data[1].file_name;
        qDebug() << change_detection_data[2].gui_name
                 << change_detection_data[2].file_name;
        read_sentinel2_bands_data(change_detection_data, m_root_path);
    } else {
        return;
    }
    QString date_time_str;
    QHash<QString, sad::geoTransform> sentinel_geo;
    if (finalFiles.empty() == false) {
        QFileInfo finfo(m_root_path + "/" + finalFiles[0] + ".jp2");
        QDir dir(finfo.absolutePath());
        dir.cdUp();
        dir.cdUp();
        const QString geo_file = dir.path() + "/MTD_TL.xml";
        fi.setFile(geo_file);
        auto xml_doc = fi.absoluteFilePath();
        change_detection_geo.utmZone =
            extractUTMZoneFromXML(xml_doc, change_detection_geo);
        date_time_str =
            getDateTimeFromXML(xml_doc).toString("yyyy/MM/dd hh:mm:s");
        sentinel_geo = extractGeoPositions(xml_doc);
        change_detection_geo.ulX = sentinel_geo["20"].ulX;
        change_detection_geo.ulY = sentinel_geo["20"].ulY;
        change_detection_geo.resX = 20;
        change_detection_geo.resY = -20;
    }
    qDebug() << "cd height" << change_detection_data[0].height;
    qDebug() << "cd width" << change_detection_data[0].width;

    // DUBLICATED CODE CHANGE DETECTION FROM COMMON SENTINEL

    qDebug() << "NIR2" << m_sentinel_data[8].gui_name;
    qDebug() << "SWIR2" << m_sentinel_data[9].gui_name;
    qDebug() << "SWIR3" << m_sentinel_data[10].gui_name;

    connect(change_detection_future, &QFutureWatcher<QPixmap>::finished, [=]() {
        QLabel *label = new QLabel;
        label->setWindowTitle(
            date_time_str + " --- " +
            m_sentinel_metadata.image_attributes.date_acquired);
        label->setAttribute(Qt::WA_DeleteOnClose);
        label->setScaledContents(true);
        label->setPixmap(change_detection_future->result());
        label->show();
        change_detection_future->deleteLater();
    });

    auto polItem = ui->graphicsView_satellite_image->getPolygonById(polygonId);
    auto br = polItem->boundingRect();
    qDebug() << "bounding TL x -- y: " << br.x() << br.y();
    qDebug() << "bounding width -- height: " << br.width() << br.height();

    change_detection_future->setFuture(QtConcurrent::run([=]() -> QPixmap {
        int nYSize = br.height();
        int nXSize = br.width();
        uchar *data = new uchar[nYSize * nXSize * 3]();

        const int xOffset = br.x();  // 2200
        const int yOffset = br.y();  // 3500
        int cd_width = change_detection_data[0].width;
        int cd_height = change_detection_data[0].height;

        // Сетка 5x5 для высокой точности (25 точек вместо 4)
        const int grid_size = 5;
        double lat_grid[grid_size][grid_size];
        double lon_grid[grid_size][grid_size];

        // Вычисление координат сетки
        for (int gy = 0; gy < grid_size; ++gy) {
            for (int gx = 0; gx < grid_size; ++gx) {
                int xValue = xOffset + (gx * nXSize) / (grid_size - 1);
                int yValue = yOffset + (gy * nYSize) / (grid_size - 1);
                getGeoCoordinates(xValue, yValue, m_geo, lat_grid[gy][gx],
                                  lon_grid[gy][gx], false);
            }
        }

        //#pragma omp parallel for num_threads( \
        //std::min(8, (int)std::thread::hardware_concurrency()))
        for (int y = 0; y < nYSize; ++y) {
            int row_offset = y * nXSize * 3;

            // Определение ячейки сетки для строки y
            int gy = (y * (grid_size - 1)) / nYSize;
            double t = (double)y / nYSize * (grid_size - 1) - gy;

            for (int x = 0; x < nXSize; ++x) {
                // Определение ячейки сетки для столбца x
                int gx = (x * (grid_size - 1)) / nXSize;
                double s = (double)x / nXSize * (grid_size - 1) - gx;

                // Билинейная интерполяция в ячейке сетки
                double lat0 =
                    lat_grid[gy][gx] * (1 - s) + lat_grid[gy][gx + 1] * s;
                double lat1 = lat_grid[gy + 1][gx] * (1 - s) +
                              lat_grid[gy + 1][gx + 1] * s;
                double lon0 =
                    lon_grid[gy][gx] * (1 - s) + lon_grid[gy][gx + 1] * s;
                double lon1 = lon_grid[gy + 1][gx] * (1 - s) +
                              lon_grid[gy + 1][gx + 1] * s;

                double lat = lat0 * (1 - t) + lat1 * t;
                double lon = lon0 * (1 - t) + lon1 * t;

                QPointF point = geoToPixel(lat, lon, change_detection_geo);
                int cdX = qBound(0, static_cast<int>(point.x()), cd_width - 1);
                int cdY = qBound(0, static_cast<int>(point.y()), cd_height - 1);
                float valueCloudMask2 = 0;
                double nir2 = 0, swir3 = 0;
                if (cdX >= 0 && cdX < cd_width && cdY >= 0 && cdY < cd_height) {
                    nir2 = change_detection_data[0].data[cdY * cd_width + cdX] -
                           1000;
                    swir3 =
                        change_detection_data[2].data[cdY * cd_width + cdX] -
                        1000;
                    valueCloudMask2 = dataCloudMask2[cdY * cd_width + cdX];
                }

                float valueCloudMask =
                    dataCloudMask[((y + yOffset) * wCloudM) + x + xOffset];

                if (valueCloudMask < 10 && valueCloudMask2 < 10) {
                    double nbr = sam::calculateNBR(nir2, swir3);

                    auto values = getKsyValues(x + xOffset, y + yOffset);
                    auto waves = getWaves();
                    auto bv = getBandsValues(waves, values, m_satelite_type);
                    double nbr_loaded = sam::calculateNBR(bv.nir2, bv.swir3);

                    nbr = nbr_loaded - nbr;

                    uchar nbr_8bit = qBound(
                        uchar(0), static_cast<uchar>((nbr + 1.0) * 127.5),
                        uchar(255));

                    int pixel_offset = row_offset + x * 3;
                    data[pixel_offset] = data[pixel_offset + 1] =
                        data[pixel_offset + 2] = nbr_8bit;
                } else {
                    int pixel_offset = row_offset + x * 3;
                    data[pixel_offset] = 0;
                    data[pixel_offset + 1] = 0;
                    data[pixel_offset + 2] = 255;
                }
            }
        }

        QImage img(data, nXSize, nYSize, nXSize * 3, QImage::Format_RGB888);
        img.save("last_change_detection.png", "PNG");
        QImage imgCopy = img;
        applyContrast(imgCopy, 0.6);
        imgCopy.save(QApplication::applicationDirPath() + "/cd_contrasted.png");
        openImageByDesktop("cd_contrasted.png");
        QPixmap pixmap = QPixmap::fromImage(img);
        delete[] data;
        delete[] dataCloudMask;
        delete[] dataCloudMask2;
        for (int i = 0; i < change_detection_data.size(); ++i) {
            if (change_detection_data[i].data)
                delete[] change_detection_data[i].data;
        }

        return pixmap;
    }));
}

QStringList MainWindowSatelliteComparator::getLandSat9BandsFromTxtFormat(
    const QString &path, QList<QString> &available_gui_bands) {
    QStringList bands;
    QFile file(path);
    if (file.exists() == false) return bands;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if (file.open(QIODevice::ReadOnly) == false) return bands;
    QString temp;
    QStringList bands_lines;
    bool isGroupProductContentsStart = false;
    while (ts.readLineInto(&temp)) {
        if (temp.contains("GROUP = PRODUCT_CONTENTS")) {
            isGroupProductContentsStart = true;
        }
        if (temp.contains("END_GROUP = PRODUCT_CONTENTS")) {
            break;
        }
        if (isGroupProductContentsStart == false) continue;
        if (temp.contains("FILE_NAME_BAND_")) {
            bands_lines.append(temp);
            temp = temp.mid(temp.indexOf('"'), temp.lastIndexOf('"'));
            temp.replace('"', "");
            bands.append(temp);
        }
    }
    // qDebug()<<bands_lines;
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        QString searchString = sad::landsat9_bands_keys[i];
        bool found = false;
        for (const QString &item : bands_lines) {
            if (item.contains(searchString, Qt::CaseSensitive)) {
                found = true;
                // qDebug()<<item<<"********** MATCHED
                // ****************"<<searchString;
                break;
            }
        }
        if (found == false) {
            qDebug() << "missed band: " << sad::landsat9_bands_keys[i];
            m_landsat9_missed_channels[i] = true;
        } else {
            m_landsat9_missed_channels[i] = false;
            available_gui_bands.append(sad::landsat_bands_gui_names[i]);
        }
    }
    return bands;
}

QString MainWindowSatelliteComparator::getLandSatSpaceCraftIDFromTxtFormat(
    const QString &path) {
    QFile file(path);
    if (file.exists() == false) return "";
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if (file.open(QIODevice::ReadOnly) == false) return "";
    QString temp;
    QStringList bands_lines;
    bool isGroupImageAttributes = false;
    while (ts.readLineInto(&temp)) {
        if (temp.contains("GROUP = IMAGE_ATTRIBUTES")) {
            isGroupImageAttributes = true;
        }
        if (temp.contains("END_GROUP = IMAGE_ATTRIBUTES")) {
            break;
        }
        if (isGroupImageAttributes == false) continue;
        if (temp.contains("SPACECRAFT_ID")) {
            bands_lines.append(temp);
            temp = temp.mid(temp.indexOf('"'), temp.lastIndexOf('"'));
            temp.replace('"', "");
            return temp;
        }
    }
    return "";
}

void MainWindowSatelliteComparator::fillLandSat9ReflectanceMultAdd(
    const QString &path) {
    QFile file(path);
    if (file.exists() == false) return;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if (file.open(QIODevice::ReadOnly) == false) return;
    QString temp;
    QVector<double> mult;
    QVector<double> add;
    bool isReflectanceGroup = false;
    while (ts.readLineInto(&temp)) {
        temp = temp.trimmed();
        if (temp == "GROUP = LEVEL2_SURFACE_REFLECTANCE_PARAMETERS") {
            isReflectanceGroup = true;
            continue;
        }
        if (isReflectanceGroup == false) continue;
        if (temp == "END_GROUP = LEVEL2_SURFACE_REFLECTANCE_PARAMETERS") {
            break;
        }
        if (temp.contains("REFLECTANCE_MULT_BAND_")) {
            temp = temp.mid(temp.indexOf("= "), temp.size() - 1);
            temp.replace("= ", "");
            mult.append(temp.toDouble());
        } else if (temp.contains("REFLECTANCE_ADD_BAND_")) {
            temp = temp.mid(temp.indexOf("= "), temp.size() - 1);
            temp.replace("= ", "");
            add.append(temp.toDouble());
        }
    }

    if (mult.size() != add.size()) {
        qDebug() << "SIZES ARE NOT THE SAME....";
        return;
    }
    if (mult.size() > LANDSAT_BANDS_NUMBER) {
        qDebug() << "SIZE TOO BIG...";
        return;
    }
    for (int i = 0; i < mult.size(); ++i) {
        m_reflectance_mult_add_arrays[i][0] = mult[i];
        m_reflectance_mult_add_arrays[i][1] = add[i];
    }
}

void MainWindowSatelliteComparator::fillLandSat9GeoData(const QString &path) {
    QFile file(path);
    if (file.exists() == false) return;
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    if (file.open(QIODevice::ReadOnly) == false) return;
    QString temp;
    bool isProjectionGroup = false;
    while (ts.readLineInto(&temp)) {
        if (temp.contains("GROUP = PROJECTION_ATTRIBUTES")) {
            isProjectionGroup = true;
            continue;
        }
        if (isProjectionGroup == false) continue;
        if (temp.contains("END_GROUP = PROJECTION_ATTRIBUTES")) {
            break;
        }
        if (temp.contains("CORNER_UL_PROJECTION_X_PRODUCT")) {
            temp = temp.mid(temp.indexOf("= "), temp.size() - 1);
            temp.replace("= ", "");
            m_geo.ulX = temp.toDouble();
        } else if (temp.contains("CORNER_UL_PROJECTION_Y_PRODUCT")) {
            temp = temp.mid(temp.indexOf("= "), temp.size() - 1);
            temp.replace("= ", "");
            m_geo.ulY = temp.toDouble();
        } else if (temp.contains("UTM_ZONE")) {
            temp = temp.mid(temp.indexOf("= "), temp.size() - 1);
            temp.replace("= ", "");
            m_geo.utmZone = temp.toDouble();
        } else if (temp.contains("GRID_CELL_SIZE_REFLECTIVE")) {
            temp = temp.mid(temp.indexOf("= "), temp.size() - 1);
            temp.replace("= ", "");
            m_geo.resX = temp.toDouble();
            m_geo.resY = -m_geo.resX;
        }
    }
}

void MainWindowSatelliteComparator::clearLandsat9DataBands() {
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        if (m_landsat9_missed_channels[i]) continue;
        if (m_landsat9_data_bands[i] == nullptr) continue;
        delete[] m_landsat9_data_bands[i];
        m_landsat9_missed_channels[i] = true;
        m_landsat9_data_bands[i] = nullptr;
    }
}

void MainWindowSatelliteComparator::cursorPointOnSceneChangedEventTimeRow(
    const QPointF &pos, const bool is_landsat) {
    if (m_time_row.empty()) return;
    int xSize = INT_MAX;
    int ySize = INT_MAX;
    for (int i = 0; i < m_time_row.size(); ++i) {
        if (xSize > m_time_row[i][0].width) xSize = m_time_row[i][0].width;
        if (ySize > m_time_row[i][0].height) ySize = m_time_row[i][0].height;
    }

    if (pos.x() >= xSize || pos.x() < 0) return;
    if (pos.y() >= ySize || pos.y() < 0) return;

    const QString x_y = "x: %1   y:%2";
    QString x_y_message =
        x_y.arg(QString::number(pos.x()), QString::number(pos.y()));
    m_label_scene_coord->setText(x_y_message);
    double latitude = 0.0;
    double longitude = 0.0;
    auto geo_coord_str = getGeoCoordinates(pos.x(), pos.y(), m_time_row_geo[0],
                                           latitude, longitude, true);
    ui->statusbar->showMessage(geo_coord_str);
    QVector<QPointF> m_points(m_time_row.size());
    for (int i = 0; i < m_time_row.size(); ++i) {
        m_points[i] = (geoToPixel(latitude, longitude, m_time_row_geo[i]));
        if (m_points[i].x() >= xSize || m_points[i].x() < 0) return;
        if (m_points[i].y() >= ySize || m_points[i].y() < 0) return;
    }
    QVector<double> waves_vi_indexes;
    QVector<double> values_vi_indexes;
    QVector<sad::BANDS_FOR_CALCULATING_INDEXES> data_indexes;
    for (int i = 0; i < m_points.size(); ++i) {
        m_viewers[i]->centerOnPoint(m_points[i]);
        uint16_t value = 0;
        QVector<double> one_ksy;
        QVector<double> waves;
        int red_band_index = 0;
        int nir_band_index = 0;
        int swir1_index = 0;

        if (is_landsat) {
            red_band_index = 3;
            nir_band_index = 4;
            swir1_index = 5;
        } else {
            red_band_index = 3;
            nir_band_index = 6;
            swir1_index = 9;
        }

        sad::BANDS_FOR_CALCULATING_INDEXES values;
        for (int j = 0; j < m_time_row[i].size(); ++j) {
            value = m_time_row[i][j]
                        .data[((int)m_points[i].y() * m_time_row[i][j].width) +
                              (int)m_points[i].x()];
            double one_ksy_value;
            if (is_landsat) {
                one_ksy_value = m_time_row[i][j].reflectance_mult * value +
                                m_time_row[i][j].reflectance_add;
            } else {
                one_ksy_value = value / 10000.0;
            }

            if (one_ksy_value == 0 || one_ksy_value > 1) continue;
            if (j == red_band_index) {
                values.RED_BAND = one_ksy_value;
            }  // red value    3 sentinel
            if (j == nir_band_index) {
                values.NIR_BAND = one_ksy_value;
            }  // nir value    6 sentinel
            if (j == swir1_index) {
                values.SWIR1_BAND = one_ksy_value;
            }  // swir1 value  9 sentinel
            one_ksy.push_back(one_ksy_value);
            waves.push_back(m_time_row[i][j].central_wave_length);
        }
        data_indexes.push_back(values);
        m_preview_plot->graph(i)->data().clear();
        m_preview_plot->graph(i)->setData(waves, one_ksy);
        m_preview_plot->rescaleAxes(true);
        m_preview_plot->replot();
        if (i == 0) {
            waves_vi_indexes = waves;
            values_vi_indexes = one_ksy;
        }
    }

    QVector<double> ndvi_time_row;
    QVector<double> ndwi_time_row;
    for (int i = 0; i < data_indexes.size(); ++i) {
        double ndvi = sam::calculateNDVI(data_indexes[i].NIR_BAND,
                                         data_indexes[i].RED_BAND);
        double ndwi = sam::calculateSWVI(data_indexes[i].NIR_BAND,
                                         data_indexes[i].SWIR1_BAND);
        ndvi_time_row.push_back(ndvi);
        ndwi_time_row.push_back(ndwi);
    }
    showTimeRowIndexesDataViaPlot(std::move(ndvi_time_row),
                                  std::move(ndwi_time_row));

    /* auto ndvi_ndwi_indexes = getIndexesForTimeRow(m_points);
    auto ndvi_vector = ndvi_ndwi_indexes.ndvi_time_row;
    qDebug()<<"----------------------------------";
    for(const auto &value: ndvi_vector){
        qDebug()<<"ndvi: "<<value;
    }
    qDebug()<<"----------------------------------";*/
    // qDebug()<<"ndvi -->"<<ndvi_ndwi_indexes.ndvi_time_row.size();
    // qDebug()<<"ndwi -->"<<ndvi_ndwi_indexes.ndvi_time_row.size();
    if (!is_landsat) {
        auto sat_type = sad::SATELLITE_TYPE::SENTINEL_2A;
        auto bv = getBandsValues(waves_vi_indexes, values_vi_indexes, sat_type);

        double dswi = sam::calculateDSWI(bv.nir1, bv.green, bv.swir2, bv.red);
        double evi = sam::calculateEVI(bv.nir1, bv.red, bv.blue);
        double ndvi = sam::calculateNDVI(bv.nir1, bv.red);
        double swvi = sam::calculateSWVI(bv.nir1, bv.swir2);

        m_spectralWidget->setIndices({{sam::kSpectralIndexNDVI, ndvi},
                                      {{sam::kSpectralIndexSWVI}, {swvi}},
                                      {{sam::kSpectralIndexDSWI}, {dswi}},
                                      {sam::kSpectralIndexEVI, evi}});
    }
}

uint16_t *MainWindowSatelliteComparator::readTiff(const QString &path,
                                                  int &xSize, int &ySize) {
    QString imgPath = path;
    QByteArray ba = imgPath.toUtf8();
    QFile file(imgPath);
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    bool isFileExists = file.exists();

    if (false == isFileExists) {
        qDebug() << "File exists: -->" << path << isFileExists;
        return nullptr;
    }
    const char *fileName = ba.constData();

    GDALDataset *poDataset = (GDALDataset *)GDALOpen(fileName, GA_ReadOnly);
    GDALRasterBand *poBand;
    poBand = poDataset->GetRasterBand(1);
    xSize = poBand->GetXSize();
    ySize = poBand->GetYSize();
    uint16_t *raster = new uint16[xSize * ySize];
    poBand->RasterIO(GF_Read, 0, 0, xSize, ySize, raster, xSize, ySize,
                     GDT_UInt16, 0, 0);
    GDALClose(poDataset);
    return raster;
}

void MainWindowSatelliteComparator::read_landsat_bands_data(
    const QStringList &file_names) {
    for (int i = 0; i < file_names.size(); ++i) {
        auto band_file_name = file_names[i];
        int xS = 0;
        int yS = 0;
        m_landsat9_data_bands[i] =
            readTiff(m_root_path + "/" + band_file_name, xS, yS);
        // qDebug()<<"x y -- sizes: "<<xS<<yS;
        m_landsat9_bands_image_sizes[i] = {xS, yS};
    }
}

QVector<double> MainWindowSatelliteComparator::getLandsat8Speya(const int x,
                                                                const int y) {
    if (m_is_image_created == false) return {};
    int xSize = m_landsat9_bands_image_sizes->first;
    int ySize = m_landsat9_bands_image_sizes->second;
    if (x > xSize || y > ySize) return {};
    if (x < 0 || y < 0) return {};
    QVector<double> speya;
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        if (i == 7 || i == 9 || i == 10)
            continue;  // пропускаем каналы panchrom, и последние два LWR-100m
        uint16_t value = m_landsat9_data_bands[i][(y * xSize) + x];
        double speya_d = m_radiance_mult_add_arrays[i][0] * value +
                         m_radiance_mult_add_arrays[i][1];
        speya.append(speya_d);
    }
    // qDebug()<<"speya: --> "<<speya;
    return speya;
}

inline QVector<double> MainWindowSatelliteComparator::getLandsat8Ksy(
    const int x, const int y) {
    if (m_is_image_created == false) return {};
    int xSize = m_landsat9_bands_image_sizes->first;
    int ySize = m_landsat9_bands_image_sizes->second;
    if (x > xSize || y > ySize) return {};
    if (x < 0 || y < 0) return {};
    QVector<double> ksy;
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        if (i == 7 || i == 8 || i == 9 || i == 10)
            continue;  // пропускаем каналы panchrom, и последние два LWR-100m
        uint16_t value = m_landsat9_data_bands[i][(y * xSize) + x];
        double ksy_d = m_reflectance_mult_add_arrays[i][0] * value +
                       m_reflectance_mult_add_arrays[i][1];
        ksy.append(ksy_d);
    }
    return ksy;
}

void MainWindowSatelliteComparator::paintSamplePoints(const QColor &color) {
    if (m_satelite_type == sad::SATELLITE_TYPE::TIME_ROW_LANDSAT_COMBINATION) {
        return;
    }
    int xSize = 0;
    int ySize = 0;
    QVector<double> sample;

    if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
        m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
        xSize = m_landsat9_bands_image_sizes->first;
        ySize = m_landsat9_bands_image_sizes->second;
        sample = m_landsat9_sample;
    } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2C) {
        if (m_sentinel_data.empty()) return;  // MESSAGE WARNING
        xSize = m_sentinel_data[0].width;
        ySize = m_sentinel_data[0].height;
        sample = m_sentinel_sample;
        if (sample.empty()) return;
    };

    int total_pixels = xSize * ySize;

    if (m_is_bekas) {
        sample = m_bekas_sample;
    }

    auto new_layer = new uchar[total_pixels * 4];

    int midY = ySize / 2;
    int offset1 = 0;
    int offset2 = midY * xSize * 4;

    std::thread t1(&MainWindowSatelliteComparator::processLayer, this,
                   new_layer, xSize, 0, midY, sample, color, offset1);
    std::thread t2(&MainWindowSatelliteComparator::processLayer, this,
                   new_layer, xSize, midY, ySize, sample, color, offset2);
    t1.join();
    t2.join();
    auto cleanup = [](void *info) { delete[] static_cast<uchar *>(info); };
    auto img = QImage(new_layer, xSize, ySize, xSize * 4,
                      QImage::Format_RGBA8888, cleanup, new_layer);
    auto pixmap = QPixmap::fromImage(img);
    auto new_image_item = new QGraphicsPixmapItem(pixmap);
    new_image_item->setZValue(
        ui->graphicsView_satellite_image->getMaxZValue(m_scene));
    m_scene->addItem(new_image_item);
    ui->graphicsView_satellite_image->centerOn(m_scene_cross_square_item);

    const QString searchParams = m_comboBox_calculation_method->currentText() +
                                 ": " +
                                 QString::number(euclid_param_spinbox->value());
    auto stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd/hh:mm:ss");
    m_layers_search_result_items.insert(stamp, new_image_item);
    m_layer_gui_list->addItemToList(stamp, searchParams, color);
}

void MainWindowSatelliteComparator::paintMultiSpecPoints(
    const QVector<int> &pixelX, const QVector<int> &pixelY,
    const QVector<QColor> &colors) {
    if (!m_image_item) {
        qWarning() << "paintMultiSpecPoints: base image item is null";
        return;
    }

    if (pixelX.isEmpty() || pixelY.isEmpty() || colors.isEmpty()) {
        qWarning() << "paintMultiSpecPoints: empty input data";
        return;
    }

    if (pixelX.size() != pixelY.size() || pixelX.size() != colors.size()) {
        qWarning() << "paintMultiSpecPoints: size mismatch"
                   << "pixelX =" << pixelX.size() << "pixelY =" << pixelY.size()
                   << "colors =" << colors.size();
        return;
    }

    const int xSize = m_satellite_image.width();
    const int ySize = m_satellite_image.height();

    if (xSize <= 0 || ySize <= 0) {
        qWarning() << "paintMultiSpecPoints: invalid image size" << xSize
                   << ySize;
        return;
    }

    auto new_layer = new uchar[xSize * ySize * 4];
    memset(new_layer, 0, xSize * ySize * 4);

    for (int i = 0; i < pixelX.size(); ++i) {
        const int x = pixelX[i];
        const int y = pixelY[i];

        if (x < 0 || y < 0 || x >= xSize || y >= ySize) {
            qWarning() << "paintMultiSpecPoints: point out of bounds" << x << y;
            continue;
        }

        const QColor &color = colors[i];
        const int offset = (y * xSize + x) * 4;

        new_layer[offset] = static_cast<uchar>(color.red());
        new_layer[offset + 1] = static_cast<uchar>(color.green());
        new_layer[offset + 2] = static_cast<uchar>(color.blue());
        new_layer[offset + 3] = 255;
    }

    auto cleanup = [](void *info) { delete[] static_cast<uchar *>(info); };
    QImage img(new_layer, xSize, ySize, xSize * 4, QImage::Format_RGBA8888,
               cleanup, new_layer);

    auto pixmap = QPixmap::fromImage(img);
    auto new_image_item = new QGraphicsPixmapItem(pixmap);
    new_image_item->setZValue(
        ui->graphicsView_satellite_image->getMaxZValue(m_scene));

    m_scene->addItem(new_image_item);
    auto stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd/hh:mm:ss");
    m_layers_search_result_items.insert(stamp, new_image_item);
    m_layer_gui_list->addItemToList(stamp, "", QColor(0, 255, 0));
    m_scene->update();
}

QString MainWindowSatelliteComparator::getGeoCoordinates(
    const int x, const int y, const sad::geoTransform &geo, double &latitude,
    double &longitude, bool isStringReturn) {
    // Создаем проекцию UTM
    OGRSpatialReference utmSrs;
    utmSrs.SetProjCS("UTM");
    utmSrs.SetWellKnownGeogCS("WGS84");  // DATUM из MTL.json
    utmSrs.SetUTM(geo.utmZone,
                  geo.isNorth);  // Северное - true или южное - false полушарие

    // Создаем целевую проекцию (WGS84)
    OGRSpatialReference wgs84Srs;
    wgs84Srs.SetWellKnownGeogCS("WGS84");

    // Создаем преобразователь координат
    OGRCoordinateTransformation *transformer =
        OGRCreateCoordinateTransformation(&utmSrs, &wgs84Srs);

    // Вычисляем координаты в проекции UTM
    double utmX = geo.ulX + x * geo.resX + y * 0;
    double utmY = geo.ulY + x * 0 + y * geo.resY;

    // Преобразуем UTM -> WGS84 (широта/долгота)
    double lon = utmX;
    double lat = utmY;
    if (!transformer->Transform(1, &lon, &lat)) {
        OCTDestroyCoordinateTransformation(transformer);
        return "";
    }
    OCTDestroyCoordinateTransformation(transformer);
    // m_lattitude = lat;
    // m_longitude = lon;
    latitude = lat;
    longitude = lon;

    if (isStringReturn) {
        QString lat_lon = QString("Широта: %1 Долгота: %2").arg(lat).arg(lon);
        return lat_lon;
    }

    return "";
}

QPointF MainWindowSatelliteComparator::geoToPixel(double latitude,
                                                  double longitude,
                                                  const sad::geoTransform &gt) {
    static OGRSpatialReference srcSRS;
    static OGRSpatialReference dstSRS;
    static OGRCoordinateTransformation *coordTransform = nullptr;

    if (!coordTransform) {
        srcSRS.SetWellKnownGeogCS("WGS84");
        dstSRS.SetUTM(gt.utmZone, gt.isNorth);
        dstSRS.SetWellKnownGeogCS("WGS84");
        coordTransform = OGRCreateCoordinateTransformation(&srcSRS, &dstSRS);
    }

    double x = longitude;
    double y = latitude;

    if (!coordTransform || !coordTransform->Transform(1, &x, &y)) {
        return QPointF(-1, -1);
    }

    int pixelX = static_cast<int>((x - gt.ulX) / gt.resX);
    int pixelY = static_cast<int>((y - gt.ulY) / gt.resY);
    return QPointF(pixelX, pixelY);
}

inline double MainWindowSatelliteComparator::euclideanDistance(
    const QVector<double> &v1, const QVector<double> &v2) noexcept {
    if (v1.size() != v2.size()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double sum = 0.0;
    const int N = static_cast<int>(v1.size());

    // Unrolled loop для C++14 (x2 быстрее)
    for (int i = 0; i < N; ++i) {
        const double diff = v1[i] - v2[i];
        sum += diff * diff;  // FMA friendly
    }

    return sum > 0.0 ? std::sqrt(sum) : 0.0;
}

inline double MainWindowSatelliteComparator::calculateSpectralAngle(
    const QVector<double> &S1, const QVector<double> &S2) {
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

    return qAcos(cosineTheta) * 180.0 / M_PI;  // Возвращаем угол в градусах
}

void MainWindowSatelliteComparator::showGoogleMap() {
    if (std::isnan(m_lattitude) || std::isnan(m_longitude)) {
        uts::showWarnigMessage("Точка на карте не выбрана.",
                               "Выберите точку на карте.");
        return;
    };
    std::string command = "start ";
    command.append(maps_utility::makeGoogleUrl(m_lattitude, m_longitude));
    system(command.c_str());
}

void MainWindowSatelliteComparator::resetColorsToDefaultRGB() {
    ui->widget_image_saturation_light_corrector->setDefaultSatLightValues();
    ui->widget_image_saturation_light_corrector->setDefaultStretchValues();
    if (m_dynamic_checkboxes_widget) {
        m_dynamic_checkboxes_widget->setRGBchannels();
        if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
            m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
            change_bands_and_show_image();
        } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
                   m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B) {
            change_bands_and_show_image(m_sentinel_data);
        }
    }
}

void MainWindowSatelliteComparator::change_bands_and_show_image() {
    auto bands = m_dynamic_checkboxes_widget->get_choosed_bands();
    int idxR = -1, idxG = -1, idxB = -1;
    for (const auto &b : bands) {
        if (b.second == RED) idxR = b.first;
        if (b.second == GREEN) idxG = b.first;
        if (b.second == BLUE) idxB = b.first;
    }
    if (idxR < 0 || idxG < 0 || idxB < 0 || idxR >= LANDSAT_BANDS_NUMBER ||
        idxG >= LANDSAT_BANDS_NUMBER || idxB >= LANDSAT_BANDS_NUMBER ||
        !m_landsat9_data_bands[idxR] || !m_landsat9_data_bands[idxG] ||
        !m_landsat9_data_bands[idxB])
        return;

    const int w = m_landsat9_bands_image_sizes[idxR].first;
    const int h = m_landsat9_bands_image_sizes[idxR].second;

    auto lowP = ui->widget_image_saturation_light_corrector->getLowPct();
    auto highP = ui->widget_image_saturation_light_corrector->getHighPct();
    auto gamma = ui->widget_image_saturation_light_corrector->getGamma();
    showRgbImage(m_landsat9_data_bands[idxR], m_landsat9_data_bands[idxG],
                 m_landsat9_data_bands[idxB], w, h, lowP, highP, gamma);
}

void MainWindowSatelliteComparator::change_bands_and_show_image(
    const QVector<sad::BAND_DATA> &band_data) {
    if (band_data.empty()) return;
    auto bands = m_dynamic_checkboxes_widget->get_choosed_bands();
    int idxR = -1, idxG = -1, idxB = -1;
    for (const auto &b : bands) {
        if (b.second == RED) idxR = b.first;
        if (b.second == GREEN) idxG = b.first;
        if (b.second == BLUE) idxB = b.first;
    }
    if (idxR < 0 || idxG < 0 || idxB < 0 || idxR >= band_data.size() ||
        idxG >= band_data.size() || idxB >= band_data.size())
        return;

    auto lowP = ui->widget_image_saturation_light_corrector->getLowPct();
    auto highP = ui->widget_image_saturation_light_corrector->getHighPct();
    auto gamma = ui->widget_image_saturation_light_corrector->getGamma();
    showRgbImage(band_data[idxR].data, band_data[idxG].data,
                 band_data[idxB].data, band_data[idxR].width,
                 band_data[idxR].height, lowP, highP, gamma);
}

void MainWindowSatelliteComparator::change_bands() {
    change_bands_and_show_image(m_sentinel_data);
}

void MainWindowSatelliteComparator::show_layer(const QString &id) {
    m_layers_search_result_items.value(id)->setVisible(true);
}

void MainWindowSatelliteComparator::hide_layer(const QString &id) {
    m_layers_search_result_items.value(id)->setVisible(false);
}

void MainWindowSatelliteComparator::remove_scene_layer(const QString &id) {
    qDebug() << "хотим удалить " << id;
    auto image_item = m_layers_search_result_items.value(id);

    if (image_item) {
        m_scene->removeItem(image_item);
        delete image_item;
        m_layers_search_result_items.remove(id);
        qDebug() << "удалили " << id;
    }
}

void MainWindowSatelliteComparator::add_roi_to_gui_list(const QString &id) {
    m_layer_roi_list->addItemToList(id, "Класс по умолчанию",
                                    QColor(Qt::yellow));
}

void MainWindowSatelliteComparator::show_roi_average(const QString &id) {
    qDebug() << "TEST ROI POLYGON INTERSECTION CONNECTION....";
    auto polItem = ui->graphicsView_satellite_image->getPolygonById(id);
    auto points = ui->graphicsView_satellite_image->getPointsInsidePolygon(
        polItem, m_image_item);
    /*qDebug() << "POINTS SIZE: " << points.size();
    for (int i = 0; i < points.size(); ++i) {
        qDebug() << points[i].x() << points[i].y();
    }*/
    auto br = polItem->boundingRect();
    qDebug() << "bounding TL x -- y: " << br.x() << br.y();
    qDebug() << "bounding width -- height: " << br.width() << br.height();
}

void MainWindowSatelliteComparator::send_roi_spectrs_to_matlab(
    const QString &id) {
    qDebug() << "слот для отправки спектрво в матлаб";

    MatlabAppController matlabApp;

    if (!matlabApp.isRunning()) {
        matlabApp.runIfNotRunning();
        uts::showWarnigMessage(
            "Внимание!",
            "Spectra classifier не был запущен. Дождитесь окончания его "
            "загрузки и повторите отправку спектров ");
        return;
    }

    // формируем спектры полигона
    auto polItem = ui->graphicsView_satellite_image->getPolygonById(id);
    auto points = ui->graphicsView_satellite_image->getPointsInsidePolygon(
        polItem, m_image_item);

    QVector<int> pixelsX(points.size());
    QVector<int> pixelsY(points.size());
    QVector<QVector<double>> specs(points.size());

    for (int i = 0; i < points.size(); ++i) {
        pixelsX[i] = points[i].x();
        pixelsY[i] = points[i].y();
        specs[i] = getKsyValues(pixelsX[i], pixelsY[i]);
    }
    auto waves = getWaves();

    // сохраняем Mat файл с данными
    QString exeDir = QCoreApplication::applicationDirPath();
    QString fullMatPath =
        exeDir + "/" + matlabAppDirRelativeName + "/" + matFileName;
    MatFilesOperator mat;
    mat.saveMultiSpecDataToMatFile(waves, pixelsX, pixelsY, specs, fullMatPath);

    // создаем json и отправляем его в matlab app
    QJsonObject params;
    params["matFilePath"] = fullMatPath;
    m_rpc->call("processMultiCamSpectra", QJsonValue(params));
}

void MainWindowSatelliteComparator::calculate_time_row_gradient(
    const QString &id) {
    if (m_time_row.empty()) {
        return;
    }

    const QString layerId = QString("DPHEATMAP_%1").arg(id);

    // Очистка старого слоя для этого же ROI:
    // remove_scene_layer удаляет со сцены и из m_layers_search_result_items,
    // а строку в GUI-списке надо удалить отдельно.
    if (m_layers_search_result_items.contains(layerId)) {
        remove_scene_layer(layerId);
        m_layer_gui_list->removeItemList(layerId);
    }

    auto gradientColors = iut::generateOrangeShades(m_time_row.size());

    qDebug() << "Time row gradient connection check....." << id;
    auto roi_item = ui->graphicsView_satellite_image->getPolygonById(id);
    if (!roi_item) {
        return;
    }

    QVector<QPointF> insidePoints;
    QRectF boundingRect =
        roi_item->mapToScene(roi_item->boundingRect()).boundingRect();
    QPolygonF polygon = roi_item->mapToScene(roi_item->polygon());

    qDebug() << "Bounding rect..." << boundingRect.left()
             << boundingRect.right();

    for (int x = static_cast<int>(boundingRect.left());
         x <= static_cast<int>(boundingRect.right()); ++x) {
        for (int y = static_cast<int>(boundingRect.top());
             y <= static_cast<int>(boundingRect.bottom()); ++y) {
            QPointF scenePoint(x, y);
            if (polygon.containsPoint(scenePoint, Qt::OddEvenFill)) {
                insidePoints.append(scenePoint);
            }
        }
    }

    int xSize = INT_MAX;
    int ySize = INT_MAX;
    for (int i = 0; i < m_time_row.size(); ++i) {
        if (xSize > m_time_row[i][0].width) {
            xSize = m_time_row[i][0].width;
        }
        if (ySize > m_time_row[i][0].height) {
            ySize = m_time_row[i][0].height;
        }
    }

    auto new_layer = new uchar[xSize * ySize * 4];
    std::memset(new_layer, 0, xSize * ySize * 4);

    for (int pi = 0; pi < insidePoints.size(); ++pi) {
        if (insidePoints[pi].x() >= xSize || insidePoints[pi].x() < 0) {
            continue;
        }
        if (insidePoints[pi].y() >= ySize || insidePoints[pi].y() < 0) {
            continue;
        }

        double latitude = 0.0;
        double longitude = 0.0;
        getGeoCoordinates(insidePoints[pi].x(), insidePoints[pi].y(),
                          m_time_row_geo[0], latitude, longitude, false);

        QVector<QPointF> mpoints(m_time_row.size());
        mpoints[0] = {insidePoints[pi].x(), insidePoints[pi].y()};

        bool badPoint = false;
        for (int i = 1; i < m_time_row.size(); ++i) {
            mpoints[i] = geoToPixel(latitude, longitude, m_time_row_geo[i]);
            if (mpoints[i].x() >= xSize || mpoints[i].x() < 0 ||
                mpoints[i].y() >= ySize || mpoints[i].y() < 0) {
                badPoint = true;
                break;
            }
        }
        if (badPoint) {
            continue;
        }

        auto ndvi_ndwi_indexes = getIndexesForTimeRow(mpoints);

        const int startColor =
            m_time_row[0][0].width * 4 * static_cast<int>(mpoints[0].y()) +
            static_cast<int>(mpoints[0].x()) * 4;

        QColor color;
        if (ndvi_ndwi_indexes.dp_ndvi == 0) {
            color = gradientColors[0];
        } else if (ndvi_ndwi_indexes.dp_ndvi == 1) {
            color = gradientColors[1];
        } else if (ndvi_ndwi_indexes.dp_ndvi == 2) {
            color = gradientColors[2];
        } else if (ndvi_ndwi_indexes.dp_ndvi == 3) {
            color = gradientColors[3];
        } else {
            continue;
        }

        new_layer[startColor] = color.red();
        new_layer[startColor + 1] = color.green();
        new_layer[startColor + 2] = color.blue();
        new_layer[startColor + 3] = 255;
    }

    auto cleanup = [](void *info) { delete[] static_cast<uchar *>(info); };
    auto img = QImage(new_layer, xSize, ySize, xSize * 4,
                      QImage::Format_RGBA8888, cleanup, new_layer);
    auto pixmap = QPixmap::fromImage(img);

    auto newimageitem = new QGraphicsPixmapItem(pixmap);
    newimageitem->setZValue(
        ui->graphicsView_satellite_image->getMaxZValue(m_scene));
    m_scene->addItem(newimageitem);

    const QColor startColor = gradientColors.first();
    const QColor endColor = gradientColors.last();
    m_layers_search_result_items.insert(layerId, newimageitem);
    m_layer_gui_list->addItemToList(
        layerId, buildGradientLegendTooltipDP(startColor, endColor),
        QColor(255, 165, 0), Qt::Checked);
}

void MainWindowSatelliteComparator::processLayer(uchar *layer, int xSize,
                                                 int yStart, int yEnd,
                                                 const QVector<double> sample,
                                                 QColor color,
                                                 int offsetStart) {
    int offset = offsetStart;
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = 0; x < xSize; ++x) {
            QVector<double> ksy;
            if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
                m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
                ksy = getLandsat8Ksy(x, y);
            } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
                       m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B ||
                       m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2C) {
                auto w_k = getSentinelKsy(x, y);
                ksy = w_k.second;
            }
            if (m_is_bekas) {
                size_t elems_to_copy = std::min(
                    static_cast<size_t>(ksy.size()),
                    static_cast<size_t>(5));  // TO DO DEFINE NUMBER OF CHANNELS
                std::vector<double> temp(ksy.begin(),
                                         ksy.begin() + elems_to_copy);
                ksy = QVector<double>::fromStdVector(temp);
            }

            double result = 999;
            if (m_comboBox_calculation_method->currentText() ==
                satc::spectral_angle) {
                result = calculateSpectralAngle(ksy, sample);
            } else if (m_comboBox_calculation_method->currentText() ==
                       satc::euclid_metrika) {
                result = euclideanDistance(ksy, sample);
            }

            layer[offset] = color.red();
            layer[offset + 1] = color.green();
            layer[offset + 2] = color.blue();
            layer[offset + 3] =
                result < euclid_param_spinbox->value() ? 255 : 0;

            offset += 4;
        }
    }
}

void MainWindowSatelliteComparator::initSentinelStructs() {
    m_sentinel_metadata.isHeaderValid = false;
    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        m_sentinel_metadata.sentinel_missed_channels[i] =
            true;  // Изначально считаем все каналы пропущенными
    }
    m_sentinel_sample = QVector<double>(12, 0.0);
    mask_for_sen2cor_data = nullptr;
}

void MainWindowSatelliteComparator::initLandsatStructs() {
    std::fill(std::begin(m_landsat9_missed_channels),
              std::end(m_landsat9_missed_channels), true);
    m_landsat9_sample = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

void MainWindowSatelliteComparator::setUpPreviewPlot() {
    m_preview_plot = new QCustomPlot_pro;
    m_preview_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                    QCP::iSelectPlottables |
                                    QCP::iSelectLegend);
    m_preview_plot->setFixedSize(400, 200);
    m_preview_plot->legend->setVisible(true);
    QCPGraph *graph_satellite = m_preview_plot->addGraph();
    m_preview_plot->xAxis->setRange(400, 2400);
    m_preview_plot->yAxis->setRange(-0.2, 1.2);
    m_preview_plot->xAxis->setLabel("Длина волны, nm");
    m_preview_plot->yAxis->setLabel("КСЯ");
    // Создаем заголовок
    title_satellite_name =
        new QCPTextElement(m_preview_plot, "", QFont("Arial", 10, QFont::Bold));
    m_preview_plot->plotLayout()->insertRow(0);
    m_preview_plot->plotLayout()->addElement(0, 0, title_satellite_name);
    graph_satellite->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    QCPGraph *graph_device = m_preview_plot->addGraph();
    graph_device->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 5));
    m_preview_plot->graph(0)->setName("Курсорный");
    m_preview_plot->graph(1)->setName("Образец для поиска");
    m_preview_plot->graph(0)->setPen(QPen(Qt::blue));
    m_preview_plot->graph(1)->setPen(QPen(Qt::red));
}

void MainWindowSatelliteComparator::setUpToolWidget() {
    QWidget *widget_tools = new QWidget(ui->graphicsView_satellite_image);
    m_comboBox_calculation_method = new QComboBox;
    m_comboBox_calculation_method->addItems(
        {satc::euclid_metrika, satc::spectral_angle});

    m_layer_gui_list = new LayerList;
    connect(m_layer_gui_list, SIGNAL(showItem(const QString)),
            SLOT(show_layer(const QString)));
    connect(m_layer_gui_list, SIGNAL(hideItem(const QString)),
            SLOT(hide_layer(const QString)));
    connect(m_layer_gui_list, SIGNAL(removeItem(const QString)),
            SLOT(remove_scene_layer(const QString)));

    m_layer_roi_list = new LayerRoiList;
    ui->verticalLayout_roi->addWidget(m_layer_roi_list);
    connect(m_layer_roi_list, SIGNAL(showItem(const QString)),
            ui->graphicsView_satellite_image,
            SLOT(show_roi_layer(const QString)));
    connect(m_layer_roi_list, SIGNAL(hideItem(const QString)),
            ui->graphicsView_satellite_image,
            SLOT(hide_roi_layer(const QString)));
    connect(m_layer_roi_list, SIGNAL(removeItem(const QString)),
            ui->graphicsView_satellite_image,
            SLOT(remove_roi_scene_layer(const QString)));
    connect(m_layer_roi_list,
            SIGNAL(roi_color_changed(const QString, const QColor)),
            ui->graphicsView_satellite_image,
            SLOT(changeRoiColor(const QString, const QColor)));
    connect(m_layer_roi_list, SIGNAL(roi_item_selected(const QString)),
            ui->graphicsView_satellite_image,
            SLOT(setRoiSelectEffect(const QString)));
    connect(m_layer_roi_list, SIGNAL(roiPolygonAverage(const QString)), this,
            SLOT(show_roi_average(const QString)));
    connect(m_layer_roi_list, SIGNAL(polygonForMatlabSelected(const QString)),
            this, SLOT(send_roi_spectrs_to_matlab(const QString)));
    connect(m_layer_roi_list, SIGNAL(createTimeRowGradient(const QString)),
            this, SLOT(calculate_time_row_gradient(const QString)));
    connect(m_layer_roi_list,
            SIGNAL(createTimeRowIndexesGradient(const QString)), this,
            SLOT(create_index_dynamic_maps(const QString)));
    connect(m_layer_roi_list, SIGNAL(changeDetectionRegion(const QString)),
            this, SLOT(runChangeDetectionMethod(const QString)));

    QHBoxLayout *tool_root_layout = new QHBoxLayout;
    QVBoxLayout *toolLayOut = new QVBoxLayout;
    tool_root_layout->addLayout(toolLayOut);
    const QSize tool_element_size(30, 30);

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
    QSpacerItem *spacer1 =
        new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QSpacerItem *spacer2 =
        new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    // tool_root_layout->addWidget(m_layer_gui_list);
    ui->verticalLayout_search_layers->addWidget(m_layer_gui_list);
    widget_tools->show();

    connect(pushbutton_centerOn, SIGNAL(clicked()), this,
            SLOT(centerSceneOnCrossSquare()));
    connect(zoomInButton, SIGNAL(clicked()), ui->graphicsView_satellite_image,
            SLOT(zoomIn()));
    connect(zoomOutButton, SIGNAL(clicked()), ui->graphicsView_satellite_image,
            SLOT(zoomOut()));
    connect(googleMap, SIGNAL(clicked()), this, SLOT(showGoogleMap()));
    connect(resetToRGB, SIGNAL(clicked()), this,
            SLOT(resetColorsToDefaultRGB()));
    connect(pushbutton_paint_samples, SIGNAL(clicked()), this,
            SLOT(findAreasUsingSelectedMetric()));
}

void MainWindowSatelliteComparator::makeConnectsForMenuActions() {
    connect(ui->actionBekas, SIGNAL(triggered()), this,
            SLOT(openBekasSpectraData()));
    connect(ui->actionOpenLandsat9Header, SIGNAL(triggered()), this,
            SLOT(openLandsat9HeaderData()));
    connect(ui->actionOpenLandsat8Header, SIGNAL(triggered()), this,
            SLOT(openLandsat8HeaderData()));
    connect(ui->actionSentinel_2A, SIGNAL(triggered()), this,
            SLOT(openSentinel2AHeaderData()));
    connect(ui->actionSentinel_2B, SIGNAL(triggered()), this,
            SLOT(openSentinel2BHeaderData()));
    connect(ui->actionSentinel_2C, SIGNAL(triggered()), this,
            SLOT(openSentinel2CHeaderData()));
    connect(ui->action_LoadTimeRow, SIGNAL(triggered()), this,
            SLOT(openTimeRowData()));

    connect(ui->action_copy_geo_coords, &QAction::triggered, this, [this]() {
        QString text = QString("%1 %2").arg(m_lattitude).arg(m_longitude);
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(text);
    });

    connect(ui->action_spectral_indicies, &QAction::triggered, this,
            [this](bool checked) {
                if (checked) {
                    m_spectralDock->show();
                } else {
                    m_spectralDock->hide();
                }
            });

    connect(ui->actionTimeRowIndices, &QAction::triggered, this,
            [this](bool checked) {
                if (checked) {
                    m_time_row_spectralIndicesDock.show();
                } else {
                    m_time_row_spectralIndicesDock.hide();
                    ui->actionTimeRowIndices->setChecked(false);
                }
            });

    // m_acDock
    connect(ui->action_atm_correction, &QAction::triggered, this,
            [this](bool checked) {
                if (checked) {
                    m_acDock->show();
                } else {
                    m_acDock->hide();
                    ui->action_atm_correction->setChecked(false);
                }
            });

    connect(ui->action_speya, &QAction::triggered, this, [this](bool checked) {
        if (checked) {
            m_speyaDock->show();
        } else {
            m_speyaDock->hide();
        }
    });

    connect(ui->action_Sentinel2_loadCloudMask, &QAction::triggered, this,
            &MainWindowSatelliteComparator::loadMaskForSentinelMenu);
    connect(ui->actionSentinel2_TOA, &QAction::triggered, this,
            &MainWindowSatelliteComparator::loadSentinelTOA);

    connect(ui->action_setCursorByGeoCoord, &QAction::triggered, this,
            &MainWindowSatelliteComparator::setCursorByGeo);
    ui->action_setCursorByGeoCoord->setShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_F));
    ui->action_setCursorByGeoCoord->setShortcutContext(Qt::WindowShortcut);

    connect(ui->action_load_external_spectr, &QAction::triggered, this,
            &MainWindowSatelliteComparator::setExternalSampleFromClipboard);
}

void MainWindowSatelliteComparator::addBaseItemsToScene() {
    m_scene_text_item_metric_value->setDefaultTextColor(Qt::black);
    m_scene_text_item_metric_value->setFont(QFont("Arial", 12));
    m_scene_text_item_metric_value->setZValue(Z_INDEX_CROSS_SQUARE_CURSOR_TEXT);
    m_scene->addItem(m_scene_text_item_metric_value);
    m_scene_cross_square_item->setPos(0, 0);
    m_scene_cross_square_item->setVisible(false);
    m_scene_cross_square_item->setZValue(Z_INDEX_CROSS_SQUARE_CURSOR);
    m_scene->addItem(m_scene_cross_square_item);
}

void MainWindowSatelliteComparator::read_sentinel2_bands_data(
    QVector<sad::BAND_DATA> &data, const QString &root_path) {
    for (int i = 0; i < data.size(); ++i) {
        const QString &band_file_name = data[i].file_name;
        int xS = 0;
        int yS = 0;
        const QString fullPath = root_path + "/" + band_file_name + ".jp2";
        // qDebug() << "FullPathToJP2" << fullPath;
        // QFileInfo fi(fullPath);
        // qDebug() << "Path to JP2 exists..." << fi.exists();
        data[i].data = readTiff(fullPath, xS, yS);
        data[i].width = xS;
        data[i].height = yS;
        if (data[i].resolution_in_pixel_meters == "R10m") {
            qDebug() << "RESOLUTION 10 TO 20";
            int outX = xS / 2;
            int outY = yS / 2;
            // Выделяем буфер вручную
            uint16_t *buffer = new uint16_t[outX * outY];
            downsample_uint16(data[i].data, buffer, xS, yS);
            delete[] data[i].data;
            data[i].data = buffer;
            data[i].width = outX;
            data[i].height = outY;
        }

        if (data[i].resolution_in_pixel_meters == "R60m") {
            qDebug() << "RESOLUTION 60 TO 20";
            int outX = xS * 3;
            int outY = yS * 3;
            // Выделяем буфер вручную
            uint16_t *buffer = new uint16_t[outX * outY];
            upsample_by_3_uint16(data[i].data, buffer, xS, yS);

            delete[] data[i].data;
            data[i].data = buffer;
            data[i].width = outX;
            data[i].height = outY;
        }

        qDebug() << "Sentinel band" << data[i].gui_name
                 << "size:" << data[i].width << data[i].height;
        // if (data[i].width != xS) qDebug() << "WRONG X";
        // if (data[i].height != yS) qDebug() << "WRONG Y";
    }
}

void MainWindowSatelliteComparator::gdal_start_driver() {
    QString dataPath = QApplication::applicationDirPath() + "/data";
    CPLSetConfigOption("GDAL_DATA", dataPath.toUtf8().constData());
    GDALAllRegister();
    CPLSetConfigOption("GDAL_NUM_THREADS", "ALL_CPUS");
    CPLSetConfigOption("OPENJPEG_NUM_THREADS", "AUTO");
    CPLSetConfigOption("GDAL_CACHEMAX", "512");
    CPLSetConfigOption("CPL_VSIL_USE_TEMP_FILE", "NO");
    CPLSetConfigOption("GDAL_JP2KAK_USE", "YES");
}

void MainWindowSatelliteComparator::gdal_close_driver() {
    GDALDestroyDriverManager();
}

QPair<QVector<double>, QVector<double>>
MainWindowSatelliteComparator::getSentinelKsy(const int x, const int y) {
    if (m_is_image_created == false) return {};
    if (m_sentinel_data.empty()) return {};
    int xSize = m_sentinel_data[0].width;
    int ySize = m_sentinel_data[0].height;
    if (x > xSize || y > ySize) return {};
    if (x < 0 || y < 0) return {};
    QVector<double> ksy;
    QVector<double> waves;
    for (int i = 0; i < m_sentinel_data.size(); ++i) {
        if (m_sentinel_data[i].height != ySize ||
            m_sentinel_data[i].width != xSize) {
            continue;
        };
        uint16_t value = m_sentinel_data[i].data[(y * xSize) + x] -
                         1000;  // BOA_ADD_OFFSET TODO READ FROM MTD
        // double ksy_d =
        // m_reflectance_mult_add_arrays[i][0]*value+m_reflectance_mult_add_arrays[i][1];
        ksy.append(value / 10000.0);
        waves.append(m_sentinel_data[i].central_wave_length);
    }
    return {waves, ksy};
}

QVector<double> MainWindowSatelliteComparator::getSentinelWaves() {
    if (m_is_image_created == false) return {};
    if (m_sentinel_data.empty()) return {};
    QVector<double> waves;
    for (int i = 0; i < m_sentinel_data.size(); ++i) {
        waves.append(m_sentinel_data[i].central_wave_length);
    }
    return waves;
}

QVector<double> MainWindowSatelliteComparator::getSentinelKsyValues(
    const int x, const int y) {
    if (m_is_image_created == false) return {};
    if (m_sentinel_data.empty()) return {};
    int xSize = m_sentinel_data[0].width;
    int ySize = m_sentinel_data[0].height;
    if (x > xSize || y > ySize) return {};
    if (x < 0 || y < 0) return {};
    QVector<double> ksy;
    for (int i = 0; i < m_sentinel_data.size(); ++i) {
        if (m_sentinel_data[i].height != ySize ||
            m_sentinel_data[i].width != xSize) {
            continue;
        };
        uint16_t value = m_sentinel_data[i].data[(y * xSize) + x];
        ksy.append((value - 1000) / 10000.0);  // OFFSET
    }
    return ksy;
}

QVector<double> MainWindowSatelliteComparator::getSen2CorKsy(const int x,
                                                             const int y) {
    if (m_is_image_created == false) return {};
    if (m_sen2cor_data.empty()) return {};
    int xSize = m_sen2cor_data[0].width;
    int ySize = m_sen2cor_data[0].height;
    if (x > xSize || y > ySize) return {};
    if (x < 0 || y < 0) return {};
    QVector<double> ksy;
    for (int i = 0; i < m_sen2cor_data.size(); ++i) {
        if (m_sen2cor_data[i].height != ySize ||
            m_sen2cor_data[i].width != xSize) {
            continue;
        };
        uint16_t value = m_sen2cor_data[i].data[(y * xSize) + x];
        ksy.append((value - 1000) / 10000.0);  // OFFSET
    }
    return ksy;
}

QVector<double> MainWindowSatelliteComparator::getSentinelSpeyaValues(
    const int x, const int y) {
    if (m_is_image_created == false) return {};
    if (m_sentinel_data.empty()) return {};
    int xSize = m_sentinel_data[0].width;
    int ySize = m_sentinel_data[0].height;
    if (x > xSize || y > ySize) return {};
    if (x < 0 || y < 0) return {};
    QVector<double> speya_values;
    for (int i = 0; i < m_sentinel_data.size(); ++i) {
        if (m_sentinel_data[i].height != ySize ||
            m_sentinel_data[i].width != xSize) {
            continue;
        };
        uint16_t value =
            m_sentinel_data[i].data[(y * xSize) + x] -
            1000;  // OFFSET TODO CHECKK ALL CODE --> READ IT FROM FILE
        double speya = sam::calculateSpeyaFromSentinelDN(
            value, m_sentinel_data[i].solar_irradiance,
            m_sentinel_metadata.cosSunZenithAngle);
        speya_values.append(speya);
    }
    return speya_values;
}

QVector<double> MainWindowSatelliteComparator::getKsyValues(const int x,
                                                            const int y) {
    QVector<double> data;
    if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
        m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
        data = getLandsat8Ksy(x, y);
    } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B) {
        data = getSentinelKsyValues(x, y);
    }
    return data;
}

QVector<double> MainWindowSatelliteComparator::getWaves() {
    QVector<double> waves;
    if (m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_8 ||
        m_satelite_type == sad::SATELLITE_TYPE::LANDSAT_9) {
        waves = waves_landsat9;
    } else if (m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2A ||
               m_satelite_type == sad::SATELLITE_TYPE::SENTINEL_2B) {
        waves = getSentinelWaves();
    }
    return waves;
}

void MainWindowSatelliteComparator::clear_satellite_data() {
    if (m_sentinel_data.empty()) return;
    for (int i = 0; i < m_sentinel_data.size(); ++i) {
        auto data = m_sentinel_data[i].data;
        if (data) delete[] data;
    }
    m_sentinel_data.clear();
}

void MainWindowSatelliteComparator::clear_all_layers() {
    if (m_layers_search_result_items.empty()) return;
    for (auto it = m_layers_search_result_items.constBegin();
         it != m_layers_search_result_items.constEnd(); ++it) {
        QString key = it.key();
        remove_scene_layer(key);
    }
    m_layer_gui_list->clear();
    m_layers_search_result_items.clear();
}

QHash<QString, sad::geoTransform>
MainWindowSatelliteComparator::extractGeoPositions(const QString &xmlFilePath) {
    QHash<QString, sad::geoTransform> positions;

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

        sad::geoTransform pos;
        pos.ulX = geoElem.firstChildElement("ULX").text().toDouble();
        pos.ulY = geoElem.firstChildElement("ULY").text().toDouble();
        positions.insert(resolution, pos);
    }

    return positions;
}

int MainWindowSatelliteComparator::extractUTMZoneFromXML(
    const QString &xmlFilePath, sad::geoTransform &gt) {
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
    // Берем самый последний символ строки
    QChar lastChar = csName.at(csName.length() - 1);
    if (lastChar == 'N') {
        gt.isNorth = true;
    } else if (lastChar == 'S') {
        gt.isNorth = false;
    }
    QRegularExpression re("zone\\s*(\\d+)");
    QRegularExpressionMatch match = re.match(csName);

    if (match.hasMatch()) {
        return match.captured(1).toInt();  // Возвращает номер зоны
    }

    qWarning() << "UTM зона не найдена в строке:" << csName;
    return -1;
}

QDateTime MainWindowSatelliteComparator::getDateTimeFromXML(
    const QString &xmlFilePath) {
    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << xmlFilePath;
        return QDateTime();
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Ошибка парсинга XML";
        file.close();
        return QDateTime();
    }
    file.close();

    QDomNodeList nodes = doc.elementsByTagName("SENSING_TIME");
    QString date_time = nodes.at(0).toElement().text();

    QDate date = QDate::fromString(date_time.mid(0, 10), "yyyy-MM-dd");
    QString timePart = date_time.mid(11, 12);
    QTime time = QTime::fromString(timePart, "HH:mm:ss.zzz");
    QDateTime dt(date, time);

    return dt;
}

void MainWindowSatelliteComparator::getKSY(const QPointF &pos,
                                           QVector<double> &waves,
                                           QVector<double> &ksy) {
    Q_UNUSED(pos)
    Q_UNUSED(waves)
    Q_UNUSED(ksy)
}

QImage MainWindowSatelliteComparator::createModifiedImage(const QImage &img,
                                                          double coefSat,
                                                          double coefLight) {
    QImage adjusted =
        img.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy();
    const int pixelCount = adjusted.width() * adjusted.height();
    QRgb *pixels = reinterpret_cast<QRgb *>(adjusted.bits());

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

    for (auto &f : futures) f.wait();
    return adjusted;
}

void MainWindowSatelliteComparator::initUdpRpcConnection() {
    // Читаем конфиг рядом с exe
    QString configPath = NETWORK_CONFIG_FILE_NAME;
    qDebug() << "Config path:" << configPath;
    qDebug() << "File exists:" << QFile::exists(configPath);
    QSettings settings(configPath, QSettings::IniFormat);

    quint16 localPort = settings.value("Network/cpp_local_port", 5001).toUInt();
    quint16 matlabPort = settings.value("Network/matlab_port", 5000).toUInt();
    QString host = settings.value("Network/host", "127.0.0.1").toString();

    m_rpc = new UdpJsonRpc(localPort, QHostAddress(host), matlabPort, this);

    // Регистрируем методы, которые может вызывать Matlab app.
    m_rpc->registerMethod("processClassifiedBecasSpectra",
                          [this](const QJsonValue &params) -> QJsonValue {
                              qDebug() << "processClassifiedBecasSpectra";
                              QVariantMap map = params.toVariant().toMap();
                              processpClassifiedBecasSpectraMatlabRequest(map);
                              return NULL;
                          });
    m_rpc->registerMethod("processClassifiedMultispec",
                          [this](const QJsonValue &params) -> QJsonValue {
                              qDebug() << "processClassifiedMultispec";
                              QVariantMap map = params.toVariant().toMap();
                              processpClassifiedMultiSpecMatlabRequest(map);
                              return NULL;
                          });

    // Подключаем сигнал получения результата к слоту обработки
    connect(m_rpc, &UdpJsonRpc::resultReady, this,
            &MainWindowSatelliteComparator::handleJsonRpcResult);
}

QVector<sad::BAND_DATA>
MainWindowSatelliteComparator::getDataFromJsonForLandsat8_9_TimeRow(
    const QString &headerName, sad::LANDSAT_METADATA_FILE &landsat_metadata,
    sad::geoTransform &gt) {
    QJsonObject jo;
    QVector<sad::BAND_DATA> bands_data;
    jsn::getJsonObjectFromFile(headerName, jo);

    QFileInfo fi(headerName);
    m_root_path = fi.path();
    const QString extension = fi.completeSuffix();

    if (extension == "json") {
        if (jo.contains("LANDSAT_METADATA_FILE")) {
            QJsonObject image_attributes =
                jsn::getValueByPath(
                    jo, {"LANDSAT_METADATA_FILE", "IMAGE_ATTRIBUTES"})
                    .toObject();
            landsat_metadata.image_attributes.spacecraft_id =
                image_attributes.value("SPACECRAFT_ID").toString();
            QString date_acquired =
                image_attributes.value("DATE_ACQUIRED").toString();
            landsat_metadata.image_attributes.date_acquired = date_acquired;
            QDate date{QDate::fromString(date_acquired, "yyyy-MM-dd")};
            QString timeStr = image_attributes.value("SCENE_CENTER_TIME")
                                  .toString()
                                  .remove('Z');

            int dotIndex = timeStr.indexOf('.');
            if (dotIndex != -1 && timeStr.length() > dotIndex + 4) {
                timeStr = timeStr.left(dotIndex + 4);
            }

            QTime time = {QTime::fromString(timeStr, "hh:mm:ss.zzz")};
            // qDebug()<<date.toString("yyyy-MM-dd")<<time.toString("---->
            // hh:mm:ss.zzz");
            QJsonValue value = jsn::getValueByPath(
                jo, {"LANDSAT_METADATA_FILE", "PRODUCT_CONTENTS"});
            QJsonValue radiance_value = jsn::getValueByPath(
                jo, {"LANDSAT_METADATA_FILE",
                     "LEVEL2_SURFACE_REFLECTANCE_PARAMETERS"});
            QJsonObject check_bands = value.toObject();
            QJsonObject radiance = radiance_value.toObject();
            QJsonObject projection =
                jsn::getValueByPath(
                    jo, {"LANDSAT_METADATA_FILE", "PROJECTION_ATTRIBUTES"})
                    .toObject();
            gt = getGeo(jo);
            landsat_metadata.projection_attributes.utm_zone =
                projection["UTM_ZONE"].toString().toDouble();
            landsat_metadata.projection_attributes
                .corner_ul_projection_x_product =
                projection["CORNER_UL_PROJECTION_X_PRODUCT"]
                    .toString()
                    .toDouble();
            landsat_metadata.projection_attributes
                .corner_ul_projection_y_product =
                projection["CORNER_UL_PROJECTION_Y_PRODUCT"]
                    .toString()
                    .toDouble();
            landsat_metadata.projection_attributes.grid_cell_size_reflective =
                projection["GRID_CELL_SIZE_REFLECTIVE"].toString().toDouble();
            QDateTime dt(date, time);
            m_time_row_dates_unix_time.first.push_back(dt.toSecsSinceEpoch());
            m_time_row_dates_unix_time.second.push_back(
                date.toString("yyyy_MM_dd"));
            for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
                auto sorted_order_index =
                    sad::sorted_landsat_bands_order_by_wavelength[i];
                if (check_bands
                        .value(sad::landsat9_bands_keys[sorted_order_index])
                        .isUndefined()) {
                    continue;
                }
                sad::BAND_DATA bd;
                auto band_file_name =
                    check_bands[sad::landsat9_bands_keys[sorted_order_index]]
                        .toString();
                int xS = 0;
                int yS = 0;
                bd.gui_name = sad::landsat_bands_gui_names[sorted_order_index];
                bd.data = readTiff(fi.path() + "/" + band_file_name, xS, yS);
                bd.width = xS;
                bd.height = yS;
                double mult_refl = radiance[sad::landsat9_mult_reflectence_keys
                                                [sorted_order_index]]
                                       .toString()
                                       .toDouble();
                double add_refl =
                    radiance
                        [sad::landsat9_add_reflectence_keys[sorted_order_index]]
                            .toString()
                            .toDouble();
                bd.reflectance_mult = mult_refl;
                bd.reflectance_add = add_refl;
                bd.central_wave_length =
                    sad::landsat_central_wavelengths[sorted_order_index];
                bands_data.append(bd);
            }
        }
    }

    return bands_data;
}

QVector<sad::BAND_DATA>
MainWindowSatelliteComparator::getDataForSentinel_TimeRow(
    const QString &headerName, sad::SATELLITE_TYPE st,
    sad::SENTINEL_METADATA &metadata, sad::geoTransform &gt) {
    QVector<sad::BAND_DATA> bands_data;
    QFile file(headerName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл Sentinel XML";
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Ошибка разбора XML";
        file.close();
        return {};
    }
    file.close();
    QFileInfo fi(headerName);
    m_root_path = fi.path();

    qDebug() << "sentinel time row rooth path: --->" << m_root_path;

    QStringList imageFiles;
    QDomNodeList imageNodes = doc.elementsByTagName("IMAGE_FILE");
    for (int i = 0; i < imageNodes.count(); ++i) {
        QDomNode node = imageNodes.at(i);
        imageFiles << node.toElement().text();
    }
    QStringList filteredFiles;

    for (const QString &file : qAsConst(imageFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            // Ищем точное вхождение ключа как часть имени файла
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                filteredFiles << file;
                break;  // нашли — переходим к следующему файлу
            }
        }
    }
    QStringList finalFiles;
    QMap<QString, QString> bestResolutionForBand;
    const QStringList priorityOrder = {"R20m", "R10m", "R60m"};

    // Для каждого band ищем путь с наивысшим приоритетом по разрешению
    for (const QString &bandKey : sad::sentinel_bands_keys) {
        for (const QString &resolution : priorityOrder) {
            for (const QString &file : qAsConst(filteredFiles)) {
                if (file.contains(resolution) &&
                    file.contains("_" + bandKey + "_")) {
                    bestResolutionForBand[bandKey] = file;
                    break;  // нашли лучший — переходим к следующему band
                }
            }
            if (bestResolutionForBand.contains(bandKey))
                break;  // если уже найден — не ищем в меньших разрешениях
        }
    }

    // Собираем финальный список
    finalFiles = bestResolutionForBand.values();

    qDebug() << finalFiles;

    for (const QString &file : qAsConst(finalFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            if (file.contains("_" + sad::sentinel_bands_keys[i] + "_")) {
                metadata.sentinel_missed_channels[i] =
                    false;  // Канал найден — не пропущен
                metadata.files[i] = file;
                break;
            }
        }
    }

    QList<QString> availableBandNames;
    QString gui_channels[SENTINEL_BANDS_NUMBER];
    double central_waves[SENTINEL_BANDS_NUMBER];
    if (st == sad::SENTINEL_2A) {
        copyQStringArray(sad::sentinel_2A_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2A_central_wave_lengths,
                  sad::sentinel_2A_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (st == sad::SENTINEL_2B) {
        copyQStringArray(sad::sentinel_2B_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2B_central_wave_lengths,
                  sad::sentinel_2B_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    }

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        if (!metadata.sentinel_missed_channels[i]) {
            if (gui_channels[i].contains("WV")) continue;
            availableBandNames << gui_channels[i];
            sad::BAND_DATA data;
            data.gui_name = gui_channels[i];
            data.central_wave_length = central_waves[i];
            data.file_name = metadata.files[i];

            bool isResolutionMissed = true;
            for (const QString &resolution : priorityOrder) {
                if (data.file_name.contains(resolution)) {
                    data.resolution_in_pixel_meters = resolution;
                    data.width =
                        sad::sentinel_resolutions.value(resolution).first;
                    data.height =
                        sad::sentinel_resolutions.value(resolution).second;
                    isResolutionMissed = false;
                    qDebug() << "r, w, h: " << data.resolution_in_pixel_meters
                             << data.width << data.height;
                    break;
                };
            }
            if (isResolutionMissed) {
                qDebug() << "<--------------------- NO RESOLUTION EXCEPTION "
                            "!!!------------------>";
                continue;
            }
            bands_data.append(data);
        }
    }

    read_sentinel2_bands_data(bands_data, m_root_path);

    QHash<QString, sad::geoTransform> sentinel_geo;
    if (finalFiles.empty() == false) {
        QFileInfo finfo(m_root_path + "/" + finalFiles[0] + ".jp2");
        QDir dir(finfo.absolutePath());
        dir.cdUp();
        dir.cdUp();
        const QString geo_file = dir.path() + "/MTD_TL.xml";
        fi.setFile(geo_file);
        auto xml_doc = fi.absoluteFilePath();
        // qDebug()<<xml_doc<<"--->"<<fi.exists();
        gt.utmZone = extractUTMZoneFromXML(xml_doc, gt);
        sentinel_geo = extractGeoPositions(xml_doc);
        QDateTime dt = getDateTimeFromXML(xml_doc);

        metadata.image_attributes.date_acquired =
            dt.toString("yyyy/MM/dd hh:mm:s");
        m_time_row_dates_unix_time.first.push_back(dt.toSecsSinceEpoch());
        m_time_row_dates_unix_time.second.push_back(dt.toString("yyyy_MM_dd"));

        gt.ulX = sentinel_geo["20"].ulX;
        gt.ulY = sentinel_geo["20"].ulY;
        gt.resX = 20;
        gt.resY = -20;
    }

    return bands_data;
}

sad::geoTransform MainWindowSatelliteComparator::getGeo(const QJsonObject &jo) {
    sad::geoTransform gt;
    QJsonObject projection = jsn::getValueByPath(jo, {"LANDSAT_METADATA_FILE",
                                                      "PROJECTION_ATTRIBUTES"})
                                 .toObject();
    gt.utmZone = projection["UTM_ZONE"].toString().toDouble();
    gt.ulX = projection["CORNER_UL_PROJECTION_X_PRODUCT"].toString().toDouble();
    gt.ulY = projection["CORNER_UL_PROJECTION_Y_PRODUCT"].toString().toDouble();
    gt.resX = projection["GRID_CELL_SIZE_REFLECTIVE"].toString().toDouble();
    gt.resY = -gt.resX;
    return gt;
}

QVector<QImage> MainWindowSatelliteComparator::get_cropedImages_for_time_row(
    const QVector<QVector<sad::BAND_DATA>> &m_time_row,
    sad::SATELLITE_TYPE st) {
    if (m_time_row.empty()) return {QImage()};

    double lowPct = ui->widget_image_saturation_light_corrector->getLowPct();
    double highPct = ui->widget_image_saturation_light_corrector->getHighPct();
    double gamma = ui->widget_image_saturation_light_corrector->getGamma();

    const int w = m_time_row[0][0].width;
    const int h = m_time_row[0][0].height;

    // --- Собираем все каналы R, G, B со всех снимков ---
    std::vector<const uint16_t *> allR, allG, allB;
    for (const auto &bands : m_time_row) {
        if (bands.size() < 4) continue;
        allR.push_back(bands[3].data);
        allG.push_back(bands[2].data);
        allB.push_back(bands[1].data);
    }

    // --- Вычисляем единые границы по всему ряду ---
    uint16_t ploR, phiR, ploG, phiG, ploB, phiB;
    computePercentileLimitsGlobal(allR, w, h, lowPct, highPct, ploR, phiR);
    computePercentileLimitsGlobal(allG, w, h, lowPct, highPct, ploG, phiG);
    computePercentileLimitsGlobal(allB, w, h, lowPct, highPct, ploB, phiB);

    // --- Рендерим каждый снимок с одинаковыми границами ---
    QVector<QImage> images;
    for (const auto &bands : m_time_row) {
        if (bands.size() < 4) continue;
        QImage img = buildRgbPercentileFixed(
            bands[3].data, bands[2].data, bands[1].data, w, h, ploR, phiR, ploG,
            phiG, ploB, phiB, gamma, nullptr);
        if (!img.isNull()) images.push_back(img);
    }
    return images;
}

void MainWindowSatelliteComparator::showTimeRowIndexesDataViaPlot(
    QVector<double> &&ndvis, QVector<double> &&ndwis) {
    if (!time_row_indexes_plot) return;
    QCPGraph *qcpg_ndvi;
    QCPGraph *qcpg_ndwi;
    if (time_row_indexes_plot->graphCount() == 0) {
        qcpg_ndvi = time_row_indexes_plot->addGraph();
        qcpg_ndwi = time_row_indexes_plot->addGraph();
        QColor ndvi_color(Qt::green);
        QColor ndwi_color(Qt::blue);
        qcpg_ndvi->setPen(QPen(ndvi_color));
        qcpg_ndwi->setPen(QPen(ndwi_color));
        qcpg_ndvi->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,
                                                   QPen(ndvi_color),
                                                   QBrush(ndvi_color), 10));
        qcpg_ndwi->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,
                                                   QPen(ndwi_color),
                                                   QBrush(ndwi_color), 10));

        qcpg_ndwi->setName("NDWI");
        qcpg_ndvi->setName("NDVI");
        QSharedPointer<QCPAxisTickerText> dateTicker(new QCPAxisTickerText);
        dateTicker->addTicks(m_time_row_dates_unix_time.first,
                             m_time_row_dates_unix_time.second);
        time_row_indexes_plot->xAxis->setTicker(dateTicker);

        auto *yTicker = new QCPAxisTickerFixed;
        yTicker->setTickStep(0.1);
        yTicker->setTickOrigin(0.0);
        time_row_indexes_plot->yAxis->setTicker(
            QSharedPointer<QCPAxisTickerFixed>(yTicker));
    }
    time_row_indexes_plot->graph(0)->data().clear();
    time_row_indexes_plot->graph(1)->data().clear();
    time_row_indexes_plot->graph(0)->setData(m_time_row_dates_unix_time.first,
                                             ndvis);
    time_row_indexes_plot->graph(1)->setData(m_time_row_dates_unix_time.first,
                                             ndwis);
    time_row_indexes_plot->xAxis->rescale(true);
    time_row_indexes_plot->replot();

    if (time_row_indexes_plot->isHidden()) time_row_indexes_plot->show();
}

bool MainWindowSatelliteComparator::isDataCloudShadow_OK(
    QVector<QPointF> &points) {
    if (points.size() != m_time_row_qa_mask.size()) {
        return false;
    }

    for (int i = 0; i < points.size(); ++i) {
        const int x = static_cast<int>(points[i].x());
        const int y = static_cast<int>(points[i].y());

        if (x < 0 || y < 0) {
            return false;
        }

        const auto &qa = m_time_row_qa_mask[i];

        if (m_satelite_type == sad::TIME_ROW_LANDSAT_COMBINATION) {
            if (!qa.data) {
                return false;
            }
            if (x >= qa.width || y >= qa.height) {
                return false;
            }

            const uint16_t value = qa.data[y * qa.width + x];

            const bool isFill = value & (1 << 0);
            const bool isCloud = value & (1 << 3);
            const bool isShadow = value & (1 << 4);

            if (isFill || isCloud || isShadow) {
                return false;
            }
        } else if (m_satelite_type == sad::TIME_ROW_SENTINEL_COMBINATION) {
            if (!qa.data || !qa.scl_data) {
                return false;
            }

            if (x >= qa.width || y >= qa.height) {
                return false;
            }
            if (x >= qa.scl_width || y >= qa.scl_height) {
                return false;
            }

            const uint16_t cloudProb = qa.data[y * qa.width + x];
            const uint16_t scl = qa.scl_data[y * qa.scl_width + x];

            if (cloudProb > 5) {
                return false;
            }

            if (scl == 3) {  // cloud shadow
                return false;
            }

            if (scl == 8 || scl == 9 || scl == 10) {  // cloud / cirrus
                return false;
            }
        }
    }

    return true;
}

void MainWindowSatelliteComparator::paintTimeRowBadForest(const QColor &color) {
    Q_UNUSED(color)
    QTime time;
    time.start();
    qDebug() << "time row paint image foo";

    if (m_time_row.empty()) return;

    int xSize = INT_MAX;
    int ySize = INT_MAX;

    for (int i = 0; i < m_time_row.size(); ++i) {
        if (xSize > m_time_row[i][0].width) xSize = m_time_row[i][0].width;
        if (ySize > m_time_row[i][0].height) ySize = m_time_row[i][0].height;
    }

    QVector<QPointF> m_points(m_time_row.size());
    double latitude = 0.0;
    double longitude = 0.0;

    auto new_layer = new uchar[xSize * ySize * 4];
    int offset = 0;

    for (int y = 0; y < ySize; ++y) {
        for (int x = 0; x < xSize; ++x) {
            base_pixel_geo->getGeoCoordinates(x, y, latitude, longitude);
            m_points[0] = QPoint(x, y);
            for (int i = 1; i < m_time_row.size(); ++i) {
                m_points[i] =
                    geoToPixel(latitude, longitude, m_time_row_geo[i]);
            }
            if (!isDataCloudShadow_OK(m_points)) {
                new_layer[offset] = color.red();
                new_layer[offset + 1] = color.green();
                new_layer[offset + 2] = color.blue();
                new_layer[offset + 3] = 255;
            } else {
                new_layer[offset] = color.red();
                new_layer[offset + 1] = color.green();
                new_layer[offset + 2] = color.blue();
                new_layer[offset + 3] = 0;
            };
            offset += 4;
        }
    }

    auto cleanup = [](void *info) { delete[] static_cast<uchar *>(info); };
    auto img = QImage(new_layer, xSize, ySize, xSize * 4,
                      QImage::Format_RGBA8888, cleanup, new_layer);
    auto pixmap = QPixmap::fromImage(img);
    auto new_image_item = new QGraphicsPixmapItem(pixmap);
    new_image_item->setZValue(
        ui->graphicsView_satellite_image->getMaxZValue(m_scene));
    m_scene->addItem(new_image_item);

    m_layers_search_result_items.insert("COMMON_TIME_ROW_MASK", new_image_item);
    m_layer_gui_list->addItemToList("COMMON_TIME_ROW_MASK", "", color);

    auto elapsedMs = time.elapsed();

    int hours = elapsedMs / (1000 * 60 * 60);
    int minutes = (elapsedMs / (1000 * 60)) % 60;
    int seconds = (elapsedMs / 1000) % 60;
    int milliseconds = elapsedMs % 1000;

    qDebug() << QString("Время выполнения: %1 ч %2 мин %3 сек %4 мс")
                    .arg(hours)
                    .arg(minutes)
                    .arg(seconds)
                    .arg(milliseconds);
}

sad::NDWI_NDVI_TIME_ROW MainWindowSatelliteComparator::getIndexesForTimeRow(
    const QVector<QPointF> &points) {
    int red_band_index = 0;
    int nir_band_index = 0;
    int swir1_index = 0;

    if (m_satelite_type == sad::TIME_ROW_LANDSAT_COMBINATION) {
        red_band_index = 3;
        nir_band_index = 4;
        swir1_index = 5;
    } else if (m_satelite_type == sad::TIME_ROW_SENTINEL_COMBINATION) {
        red_band_index = 3;
        nir_band_index = 6;
        swir1_index = 9;
    }

    QVector<sad::BANDS_FOR_CALCULATING_INDEXES> data_indexes;
    for (int i = 0; i < points.size(); ++i) {
        sad::BANDS_FOR_CALCULATING_INDEXES values;
        for (int j = 0; j < m_time_row[i].size(); ++j) {
            double value =
                m_time_row[i][j]
                    .data[((int)points[i].y() * m_time_row[i][j].width) +
                          (int)points[i].x()];
            double one_ksy_value = m_time_row[i][j].reflectance_mult * value +
                                   m_time_row[i][j].reflectance_add;

            if (m_satelite_type == sad::TIME_ROW_LANDSAT_COMBINATION) {
                one_ksy_value = m_time_row[i][j].reflectance_mult * value +
                                m_time_row[i][j].reflectance_add;
            } else if (m_satelite_type == sad::TIME_ROW_SENTINEL_COMBINATION) {
                one_ksy_value = value / 10000.0;
            }

            if (one_ksy_value == 0) continue;
            if (j == red_band_index) {
                values.RED_BAND = one_ksy_value;
            }
            if (j == nir_band_index) {
                values.NIR_BAND = one_ksy_value;
            }
            if (j == swir1_index) {
                values.SWIR1_BAND = one_ksy_value;
            }
        }
        data_indexes.push_back(values);
    }
    int common_size = data_indexes.size();
    QVector<double> ndvi_time_row(common_size);
    QVector<double> ndwi_time_row(common_size);
    for (int i = 0; i < common_size; ++i) {
        double ndvi = sam::calculateNDVI(data_indexes[i].NIR_BAND,
                                         data_indexes[i].RED_BAND);
        double ndwi = sam::calculateSWVI(data_indexes[i].NIR_BAND,
                                         data_indexes[i].SWIR1_BAND);
        ndvi_time_row[i] = ndvi;
        ndwi_time_row[i] = ndwi;
    }

    sad::NDWI_NDVI_TIME_ROW result;
    int dp_ndvi = 0;
    QVector<double> slopes;
    for (int i = ndvi_time_row.size() - 1; i > 0; --i) {
        QVector<double> time_frame =
            ndvi_time_row.mid(i - 1, ndvi_time_row.size() - i + 1);
        // std::reverse(time_frame.begin(), time_frame.end());
        // qDebug()<<"-------time frame-------";
        /*for(int j=0;j<time_frame.size();++j){
           qDebug()<<time_frame[j];
       }
       qDebug()<<"----end time frame----------";*/
        double slope = calculate_slope(time_frame);
        if (slope < 0) ++dp_ndvi;
        slopes.push_back(slope);
    }

    result.slopes = std::move(slopes);
    result.dp_ndvi = dp_ndvi;
    result.ndvi_time_row = std::move(ndvi_time_row);
    result.ndwi_time_row = std::move(ndwi_time_row);

    return result;
}

double MainWindowSatelliteComparator::calculate_slope(
    const QVector<double> &values) {
    LeastSquareSolver solver;
    double result = 0.0;
    std::vector<double> x_values(values.size());
    std::iota(x_values.begin(), x_values.end(), 1.0);
    solver.setModel(
        [](double x, const std::vector<double> &p) { return p[0] * x + p[1]; },
        2);
    solver.setData(x_values, values.toStdVector());
    solver.setInitialGuess({1.0, 1.0, 9.0});
    if (solver.solve()) {
        auto params = solver.getParameters();
        result = params[0];
        // qDebug() << "Slope:" << params[0] << "Intercept:" << params[1];
    }
    return result;
}

void MainWindowSatelliteComparator::setUpUi() {
    // Создаем спектральный виджет
    m_spectralWidget = new SpectralIndicesWidget();

    // Создаем док
    m_spectralDock = new QDockWidget("Спектральные индексы", this);
    m_spectralDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_spectralDock->setFeatures(QDockWidget::DockWidgetMovable |
                                QDockWidget::DockWidgetFloatable);
    m_spectralDock->setWidget(m_spectralWidget);

    m_speyaDock = new QDockWidget("СПЭЯ", this);
    m_speyaDock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_speyaDock->setFeatures(QDockWidget::DockWidgetMovable |
                             QDockWidget::DockWidgetFloatable);
    m_speyaDock->setWidget(m_speya_plot);
    m_speyaDock->setMinimumSize(QSize(400, 150));
    m_speya_plot->yAxis->setLabel("СПЭЯ Вт/(м²·мкм·ср)");
    m_speya_plot->xAxis->setLabel("Длина волны, нм");
    m_speya_plot->addGraph();

    m_acDock = new QDockWidget("Атмосферная коррекция");
    m_acDock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_acDock->setFeatures(QDockWidget::DockWidgetMovable |
                          QDockWidget::DockWidgetFloatable);
    m_acDock->setWidget(&m_ac);

    // Добавляем доки в левую область
    addDockWidget(Qt::RightDockWidgetArea, m_spectralDock);
    addDockWidget(Qt::RightDockWidgetArea, m_speyaDock);
    addDockWidget(Qt::RightDockWidgetArea, m_acDock);

    setWindowTitle("Спектральный анализатор");
    resize(1200, 800);

    time_row_indexes_plot->setMinimumSize(QSize(400, 150));
    time_row_indexes_plot->setWindowTitle("Индексы NDVI, NDWI");
    time_row_indexes_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                           QCP::iSelectPlottables |
                                           QCP::iSelectLegend);
    time_row_indexes_plot->xAxis->setTickLabelRotation(90);
    time_row_indexes_plot->xAxis->setTickLabelPadding(8);
    time_row_indexes_plot->legend->setVisible(true);
    time_row_indexes_plot->yAxis->setLabel("Значение индексов");
    time_row_indexes_plot->xAxis->setLabel("Дата съёмки");
    time_row_indexes_plot->plotLayout()->setMargins(
        QMargins(0, 10, 50, 10));  // left, top, right, bottom
    time_row_indexes_plot->yAxis->setRange(-1, 1);
    m_time_row_spectralIndicesDock.setAllowedAreas(Qt::RightDockWidgetArea);
    m_time_row_spectralIndicesDock.setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_time_row_spectralIndicesDock.setWidget(time_row_indexes_plot);
    m_time_row_spectralIndicesDock.setWindowTitle("Временной ряд индексов");
    addDockWidget(Qt::RightDockWidgetArea, &m_time_row_spectralIndicesDock);
}

void MainWindowSatelliteComparator::deleteTimeRowData() {
    if (m_time_row.empty()) return;
    for (int i = 0; i < m_time_row_qa_mask.size(); ++i) {
        if (m_time_row_qa_mask[i].data) {
            delete[] m_time_row_qa_mask[i].data;
            m_time_row_qa_mask[i].data = nullptr;
        }

        if (m_time_row_qa_mask[i].scl_data) {
            delete[] m_time_row_qa_mask[i].scl_data;
            m_time_row_qa_mask[i].scl_data = nullptr;
        }
    }
    m_time_row.clear();
}

uint16_t *MainWindowSatelliteComparator::loadMaskForSentinel(
    int &width, int &height, const QString &rootPath) {
    qDebug() << rootPath;
    // Проверка: существует ли папка
    if (!QDir(rootPath).exists() || m_root_path.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка не найдена: " + rootPath);
        return nullptr;
    }
    // MSK_CLDPRB_20m.jp2

    // QDesktopServices::openUrl(QUrl::fromLocalFile(m_root_path));
    QString pathToCloudMsk =
        satc::getPathToCloudMaskForSentinel(rootPath + "/manifest.safe");
    if (pathToCloudMsk.isEmpty()) {
        qDebug() << "файл маски не найден...";
        return nullptr;
    }
    qDebug() << "--->CLOUDMASK_FILE: " << pathToCloudMsk;

    pathToCloudMsk.replace(0, 1, rootPath);
    auto data = readTiff(pathToCloudMsk, width, height);
    qDebug() << "CLOUD MASK" << width << "--" << height;
    return data;
}

uint16_t *MainWindowSatelliteComparator::loadSCLForSentinel(
    int &width, int &height, const QString &rootPath) {
    width = 0;
    height = 0;

    QDirIterator it(rootPath, QStringList() << "*SCL_20m.jp2", QDir::Files,
                    QDirIterator::Subdirectories);

    QString sclPath;
    if (it.hasNext()) {
        sclPath = it.next();
    }

    if (sclPath.isEmpty()) {
        qDebug() << "SCL_20m.jp2 not found in:" << rootPath;
        return nullptr;
    }

    qDebug() << "SCL path:" << sclPath;
    return readTiff(sclPath, width, height);
}

bool MainWindowSatelliteComparator::saveSentinelToGeoTiff(
    const QVector<sad::BAND_DATA> &bands, const sad::geoTransform &gt,
    const QString &outputFilePath) {
    if (bands.isEmpty() || !bands[0].data) {
        qDebug() << "Error: No bands or data provided.";
        return false;
    }

    // 1. Инициализация GDAL
    GDALAllRegister();
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!poDriver) {
        qDebug() << "Error: GeoTIFF driver not found.";
        return false;
    }

    // 2. Подготовка опций создания (сжатие LZW для 16-битных данных)
    char **papszOptions = nullptr;
    papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
    papszOptions = CSLSetNameValue(papszOptions, "TILED", "YES");
    papszOptions = CSLSetNameValue(papszOptions, "BIGTIFF", "IF_NEEDED");

    // 3. Создание набора данных (Dataset)
    // Берем размеры из первого канала
    int nXSize = bands[0].width;
    int nYSize = bands[0].height;
    int nBands = bands.size();

    GDALDataset *poDstDS =
        poDriver->Create(outputFilePath.toLocal8Bit().constData(), nXSize,
                         nYSize, nBands, GDT_UInt16, papszOptions);

    if (!poDstDS) {
        qDebug() << "Error: Could not create output file.";
        CSLDestroy(papszOptions);
        return false;
    }

    // 4. Установка ГЕОПРИВЯЗКИ (из вашей структуры)
    double adfGeoTransform[6];
    adfGeoTransform[0] = gt.ulX;
    adfGeoTransform[1] = gt.resX;
    adfGeoTransform[2] = gt.rotateX;
    adfGeoTransform[3] = gt.ulY;
    adfGeoTransform[4] = gt.rotateY;
    adfGeoTransform[5] = gt.resY;
    poDstDS->SetGeoTransform(adfGeoTransform);

    // 5. Установка ПРОЕКЦИИ (WGS84 UTM)
    OGRSpatialReference oSRS;
    // Предполагаем северное полушарие (1), так как это стандарт для
    // большинства данных Sentinel
    oSRS.SetUTM(static_cast<int>(gt.utmZone), 1);
    oSRS.SetWellKnownGeogCS("WGS84");

    char *pszWKT = nullptr;
    oSRS.exportToWkt(&pszWKT);
    poDstDS->SetProjection(pszWKT);
    CPLFree(pszWKT);

    // 6. ЗАПИСЬ ДАННЫХ
    for (int i = 0; i < nBands; ++i) {
        GDALRasterBand *poBand = poDstDS->GetRasterBand(i + 1);

        // Установка имени канала для удобства в QGIS/ArcGIS
        poBand->SetDescription(bands[i].gui_name.toLocal8Bit().constData());

        // Записываем массив целиком
        CPLErr err =
            poBand->RasterIO(GF_Write, 0, 0, nXSize, nYSize, bands[i].data,
                             nXSize, nYSize, GDT_UInt16, 0, 0);

        if (err != CE_None) {
            qDebug() << "Error writing band" << i + 1;
        }
    }

    // 7. Закрытие и очистка
    GDALClose(poDstDS);
    CSLDestroy(papszOptions);

    qDebug() << "File saved successfully:" << outputFilePath;
    return true;
}

void MainWindowSatelliteComparator::loadMaskForSentinelMenu() {
    int width, height;
    uint16_t *raster = nullptr;
    if (m_root_path.isEmpty()) return;
    raster = loadMaskForSentinel(width, height, m_root_path);

    qDebug() << "Cloud mask: " << width << " -- " << height;
    // Создаём QImage (Format_Grayscale8)
    QImage image(width, height, QImage::Format_Grayscale8);
    uchar *scanline = image.bits();
    int bytesPerLine = image.bytesPerLine();

    for (int y = 0; y < height; ++y) {
        uchar *line = scanline + y * bytesPerLine;
        const uint16_t *srcRow = raster + y * width;

        for (int x = 0; x < width; ++x) {
            uint16_t val = srcRow[x];
            if (val != 0) qDebug() << val;
            // Ограничиваем диапазон [0, 100]
            if (val > 100) {
                val = 100;
                qDebug() << "More Than 100";
            }

            // Конвертируем 0–100 → 0–255
            line[x] = static_cast<uchar>((val * 255) / 100);
        }
    }

    // Передаём создание виджета в GUI поток
    QMetaObject::invokeMethod(
        this,
        [image]() {
            QLabel *label = new QLabel;
            label->setPixmap(QPixmap::fromImage(image));
            label->setAttribute(Qt::WA_DeleteOnClose);
            label->setWindowTitle("Маска облаков");
            label->setScaledContents(true);
            QSize targetSize(800, 600);
            QSize scaledSize =
                image.size().scaled(targetSize, Qt::KeepAspectRatio);

            label->resize(scaledSize);
            label->show();
        },
        Qt::QueuedConnection);
}

void MainWindowSatelliteComparator::sendSpectrToMatlab() {
    if (!m_preview_plot || m_preview_plot->graphCount() <= 1) return;

    const QCPGraph *graph =
        m_preview_plot->graph(1);  // зафиксированный образец
    if (!graph || graph->dataCount() == 0) return;

    MatlabAppController matlabApp;
    if (!matlabApp.isRunning()) {
        matlabApp.runIfNotRunning();
        uts::showWarnigMessage(
            "Внимание!",
            "Spectra classifier не был запущен. Дождитесь окончания его "
            "загрузки и повторите отправку спектра.");
        return;
    }

    QVector<double> waves, spectr;
    waves.reserve(graph->dataCount());
    spectr.reserve(graph->dataCount());

    for (auto it = graph->data()->constBegin(); it != graph->data()->constEnd();
         ++it) {
        waves.append(it->key);
        spectr.append(it->value);
    }

    QString fullMatPath = QCoreApplication::applicationDirPath() + "/" +
                          matlabAppDirRelativeName + "/" + matFileName;

    MatFilesOperator mat;
    mat.saveSingleSpectrToMatFile(waves, spectr, fullMatPath);

    QJsonObject params;
    params["matFilePath"] = fullMatPath;
    m_rpc->call("processSingleSpectr", QJsonValue(params));
}

void MainWindowSatelliteComparator::loadSentinelTOA() {
    QString headerName =
        getPathToSentinelHeader(this, satc::satellite_name_sentinel_2A_TOA);
    ui->graphicsView_satellite_image->setIsSignal(false);
    clearLandsat9DataBands();
    clear_satellite_data();
    clear_all_layers();
    deleteTimeRowData();
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
    m_sentinel_metadata.solar_irradiance =
        satc::extractSolarIrradianceForSentinel(headerName);
    const QString satelliteType =
        satc::extractSpacecraftName(headerName).toUpper();
    qDebug() << "Satellite name: " << satelliteType;
    m_satelite_type = sad::SENTINEL_2A;
    QString sentinelTOAname;
    if (satelliteType == satc::satellite_name_sentinel_2A) {
        m_satelite_type = sad::SENTINEL_2A;
        sentinelTOAname = satc::satellite_name_sentinel_2A_TOA;
        m_ac.updateSatelliteType("sentinel 2A");
    } else if (satelliteType == satc::satellite_name_sentinel_2B) {
        m_satelite_type = sad::SENTINEL_2B;
        sentinelTOAname = satc::satellite_name_sentinel_2B_TOA;
        m_ac.updateSatelliteType("sentinel 2B");
    } else if (satelliteType == satc::satellite_name_sentinel_2C) {
        m_satelite_type = sad::SENTINEL_2C;
        sentinelTOAname = satc::satellite_name_sentinel_2C_TOA;
        m_ac.updateSatelliteType("sentinel 2C");
    }
    title_satellite_name->setText(sentinelTOAname);
    QFileInfo fi(headerName);
    m_root_path = fi.path();
    m_sentinel_metadata.root_path = fi.path();
    QString dataLoadingMessage = QString("Загрузка данных %1...")
                                     .arg(satc::satellite_name_sentinel_2A_TOA);
    ui->statusbar->showMessage(dataLoadingMessage);
    QApplication::processEvents();

    QStringList imageFiles;
    QDomNodeList imageNodes = doc.elementsByTagName("IMAGE_FILE");
    for (int i = 0; i < imageNodes.count(); ++i) {
        QDomNode node = imageNodes.at(i);
        imageFiles << node.toElement().text();
    }

    QStringList filteredFiles;

    for (const QString &file : qAsConst(imageFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            // if (i != 1 && i != 2 && i != 3) continue;
            //  Ищем точное вхождение ключа как часть имени файла
            if (file.contains("_" + sad::sentinel_bands_keys[i])) {
                filteredFiles << file;
                break;  // нашли — переходим к следующему файлу
            }
        }
    }
    qDebug() << "filtered files:" << filteredFiles;

    // title_satellite_name->setText(satellite_name);

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        QString target = "_" + sad::sentinel_bands_keys[i];
        QStringList list = filteredFiles.filter(target);
        if (list.size() == 1) {
            m_sentinel_metadata.sentinel_missed_channels[i] = false;
            m_sentinel_metadata.files[i] = list.at(0);
            qDebug() << m_sentinel_metadata.files[i];
        } else if (list.size() > 1) {
            qDebug() << "DUBLICATED FILES IN FINAL FILES LIST...";
        }
        target = "";
    }

    if (m_dynamic_checkboxes_widget) m_dynamic_checkboxes_widget->clear();
    QList<QString> availableBandNames;
    QString gui_channels[SENTINEL_BANDS_NUMBER];
    double central_waves[SENTINEL_BANDS_NUMBER];
    if (m_satelite_type == sad::SENTINEL_2A) {
        copyQStringArray(sad::sentinel_2A_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2A_central_wave_lengths,
                  sad::sentinel_2A_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (m_satelite_type == sad::SENTINEL_2B) {
        copyQStringArray(sad::sentinel_2B_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2B_central_wave_lengths,
                  sad::sentinel_2B_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (m_satelite_type == sad::SENTINEL_2C) {
        copyQStringArray(sad::sentinel_2C_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2C_central_wave_lengths,
                  sad::sentinel_2C_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    }

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        if (!m_sentinel_metadata.sentinel_missed_channels[i]) {
            sad::BAND_DATA data;
            // if (gui_channels[i].contains("WV")) continue;
            availableBandNames << gui_channels[i];
            data.solar_irradiance = m_sentinel_metadata.solar_irradiance[i];
            data.gui_name = gui_channels[i];
            data.central_wave_length = central_waves[i];
            data.file_name = m_sentinel_metadata.files[i];
            data.resolution_in_pixel_meters =
                sad::sentinel_resolution_by_index[i];
            qDebug() << "file name: -->" << data.file_name;
            m_sentinel_data.append(data);
        }
    }
    m_dynamic_checkboxes_widget = new DynamicCheckboxWidget(
        availableBandNames, ui->verticalLayout_satellite_bands);
    m_dynamic_checkboxes_widget->setInitialCheckBoxesToggled({1, 2, 3});

    connect(m_dynamic_checkboxes_widget, SIGNAL(choosed_bands_changed()), this,
            SLOT(change_bands()));

    read_sentinel2_bands_data(m_sentinel_data, m_root_path);
    change_bands_and_show_image(m_sentinel_data);

    ui->statusbar->showMessage("");
    m_is_image_created = true;
    m_scene_cross_square_item->setVisible(true);
    ui->graphicsView_satellite_image->setIsSignal(true);
    QHash<QString, sad::geoTransform> sentinel_geo;
    if (filteredFiles.empty() == false) {
        QFileInfo finfo(m_root_path + "/" + filteredFiles[0] + ".jp2");
        QDir dir(finfo.absolutePath());
        dir.cdUp();
        const QString geo_file = dir.path() + "/MTD_TL.xml";
        fi.setFile(geo_file);
        auto xml_doc = fi.absoluteFilePath();
        qDebug() << xml_doc << "--->" << fi.exists();
        m_geo.utmZone = extractUTMZoneFromXML(xml_doc, m_geo);
        sentinel_geo = extractGeoPositions(xml_doc);
        m_geo.ulX = sentinel_geo["20"].ulX;
        m_geo.ulY = sentinel_geo["20"].ulY;
        m_geo.resX = 20;
        m_geo.resY = -20;

        QString date_time =
            getDateTimeFromXML(xml_doc).toString("yyyy/MM/dd hh:mm:ss");

        m_sentinel_metadata.image_attributes.date_acquired = date_time;
        m_label_date_time->setText(date_time);
        // TODO CHECK AND WARN USER ABOUT WRONG VALUES AND ERRORS
        double sunZenitAngle = satc::getSunZenitAngleForSentinel(xml_doc);
        double sunAzimutAngle = satc::getSunAzimuthAngleForSentinel(xml_doc);
        double meanZenitCaptureAngle =
            satc::getAverageCaptureZenitAngle(xml_doc);
        double meanAzimutCaptureAngle =
            satc::getAverageCaptureAzimutAngle(xml_doc);
        m_ac.setSunZenitAngle(sunZenitAngle);
        m_ac.setSunAzimutAngle(sunAzimutAngle);
        m_ac.setCaptureZenitAngle(meanZenitCaptureAngle);
        m_ac.setCaptureAzimutAngle(meanAzimutCaptureAngle);
        m_sentinel_metadata.sunZenithAngle = sunZenitAngle;
        m_sentinel_metadata.sunAzimuthAngle = sunAzimutAngle;
        m_sentinel_metadata.cosSunZenithAngle =
            cos(M_PI / 180.0 * sunZenitAngle);
        qDebug() << "sza" << sunZenitAngle << "saa" << sunAzimutAngle
                 << "cosSun" << m_sentinel_metadata.cosSunZenithAngle;
    }

    // saveSentinelToGeoTiff(m_sentinel_data, m_geo, "test2.tiff");
}

// WIP (подгрузка данных Sentinel c коррекцией sen2cor для сравнения)
void MainWindowSatelliteComparator::loadSentinelSen2Cor() {
    QString headerName = getPathToSentinelHeader(this, "Sen2Cor");

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
    m_sentinel_metadata_for_sen2cor.solar_irradiance =
        satc::extractSolarIrradianceForSentinel(headerName);
    const QString satelliteType =
        satc::extractSpacecraftName(headerName).toUpper();
    qDebug() << "Satellite name: " << satelliteType;
    sad::SATELLITE_TYPE satelite_type = sad::SENTINEL_2A;

    if (satelliteType == satc::satellite_name_sentinel_2A) {
        satelite_type = sad::SENTINEL_2A;
    } else if (satelliteType == satc::satellite_name_sentinel_2B) {
        satelite_type = sad::SENTINEL_2B;
    } else if (satelliteType == satc::satellite_name_sentinel_2C) {
        satelite_type = sad::SENTINEL_2C;
    }

    QFileInfo fi(headerName);
    const QString root_path = fi.path();
    m_sentinel_metadata_for_sen2cor.root_path = fi.path();

    QStringList imageFiles;
    QDomNodeList imageNodes = doc.elementsByTagName("IMAGE_FILE");
    for (int i = 0; i < imageNodes.count(); ++i) {
        QDomNode node = imageNodes.at(i);
        imageFiles << node.toElement().text();
    }

    QStringList filteredFiles;

    for (const QString &file : qAsConst(imageFiles)) {
        for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
            // if (i != 1 && i != 2 && i != 3) continue;
            //  Ищем точное вхождение ключа как часть имени файла
            if (file.contains("_" + sad::sentinel_bands_keys[i])) {
                filteredFiles << file;
                break;  // нашли — переходим к следующему файлу
            }
        }
    }
    qDebug() << "filtered files:" << filteredFiles;

    QStringList finalFiles;
    QMap<QString, QString> bestResolutionForBand;
    const QStringList priorityOrder = {"R20m", "R10m", "R60m"};

    // Для каждого band ищем путь с наивысшим приоритетом по разрешению
    for (const QString &bandKey : sad::sentinel_bands_keys) {
        for (const QString &resolution : priorityOrder) {
            for (const QString &file : qAsConst(filteredFiles)) {
                if (file.contains(resolution) &&
                    file.contains("_" + bandKey + "_")) {
                    bestResolutionForBand[bandKey] = file;
                    break;  // нашли лучший — переходим к следующему band
                }
            }
            if (bestResolutionForBand.contains(bandKey))
                break;  // если уже найден — не ищем в меньших разрешениях
        }
    }

    // Собираем финальный список
    finalFiles = bestResolutionForBand.values();
    qDebug() << "final files: " << finalFiles;

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        QString target = "_" + sad::sentinel_bands_keys[i] + "_";
        QStringList list = finalFiles.filter(target);
        if (list.size() == 1) {
            m_sentinel_metadata_for_sen2cor.sentinel_missed_channels[i] = false;
            m_sentinel_metadata_for_sen2cor.files[i] = list.at(0);
        } else if (list.size() > 1) {
            qDebug() << "DUBLICATED FILES IN FINAL FILES LIST...";
        }
        target = "";
    }

    QList<QString> availableBandNames;
    QString gui_channels[SENTINEL_BANDS_NUMBER];
    double central_waves[SENTINEL_BANDS_NUMBER];
    if (satelite_type == sad::SENTINEL_2A) {
        copyQStringArray(sad::sentinel_2A_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2A_central_wave_lengths,
                  sad::sentinel_2A_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (satelite_type == sad::SENTINEL_2B) {
        copyQStringArray(sad::sentinel_2B_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2B_central_wave_lengths,
                  sad::sentinel_2B_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    } else if (satelite_type == sad::SENTINEL_2C) {
        copyQStringArray(sad::sentinel_2C_gui_band_names, gui_channels,
                         SENTINEL_BANDS_NUMBER);
        std::copy(sad::sentinel_2C_central_wave_lengths,
                  sad::sentinel_2C_central_wave_lengths + SENTINEL_BANDS_NUMBER,
                  central_waves);
    }

    for (int i = 0; i < SENTINEL_BANDS_NUMBER; ++i) {
        if (!m_sentinel_metadata_for_sen2cor.sentinel_missed_channels[i]) {
            sad::BAND_DATA data;
            // if (gui_channels[i].contains("WV")) continue;
            availableBandNames << gui_channels[i];
            data.gui_name = gui_channels[i];
            data.solar_irradiance =
                m_sentinel_metadata_for_sen2cor.solar_irradiance[i];
            data.central_wave_length = central_waves[i];
            data.file_name = m_sentinel_metadata_for_sen2cor.files[i];

            bool isResolutionMissed = true;
            for (const QString &resolution : priorityOrder) {
                if (data.file_name.contains(resolution)) {
                    data.resolution_in_pixel_meters = resolution;
                    data.width =
                        sad::sentinel_resolutions.value(resolution).first;
                    data.height =
                        sad::sentinel_resolutions.value(resolution).second;
                    isResolutionMissed = false;
                    qDebug() << "r, w, h: " << data.resolution_in_pixel_meters
                             << data.width << data.height;
                    break;
                };
            }
            if (isResolutionMissed) {
                // TODO EXCEPTION
                // Мы обязательно должны знать разрешение
                //  Выбросить исключение
                qDebug() << "<--------------------- NO RESOLUTION EXCEPTION "
                            "!!!------------------>";
            }
            m_sen2cor_data.append(data);
        }
    }

    read_sentinel2_bands_data(m_sen2cor_data, root_path);

    QHash<QString, sad::geoTransform> sentinel_geo;
    if (filteredFiles.empty() == false) {
        QFileInfo finfo(root_path + "/" + filteredFiles[0] + ".jp2");
        QDir dir(finfo.absolutePath());
        dir.cdUp();
        dir.cdUp();
        const QString geo_file = dir.path() + "/MTD_TL.xml";
        fi.setFile(geo_file);
        auto xml_doc = fi.absoluteFilePath();
        qDebug() << xml_doc << "--->" << fi.exists();
        m_sen2cor_geo.utmZone = extractUTMZoneFromXML(xml_doc, m_sen2cor_geo);
        sentinel_geo = extractGeoPositions(xml_doc);
        m_sen2cor_geo.ulX = sentinel_geo["20"].ulX;
        m_sen2cor_geo.ulY = sentinel_geo["20"].ulY;
        m_sen2cor_geo.resX = 20;
        m_sen2cor_geo.resY = -20;

        QString date_time =
            getDateTimeFromXML(xml_doc).toString("yyyy/MM/dd hh:mm:ss");

        m_sentinel_metadata_for_sen2cor.image_attributes.date_acquired =
            date_time;
    }
    mask_for_sen2cor_data = loadSCLForSentinel(
        m_sen2cor_data[0].width, m_sen2cor_data[0].height, root_path);
}

void MainWindowSatelliteComparator::setCursorByGeo() {
    GeoPointFinder *gpf = new GeoPointFinder;
    gpf->setAttribute(Qt::WA_DeleteOnClose);
    gpf->show();
    connect(gpf, &GeoPointFinder::setGeoCoordinatesAsSample, this,
            [this](QPointF latLon) {
                sad::geoTransform geo;
                if (m_satelite_type != sad::TIME_ROW_LANDSAT_COMBINATION &&
                    m_satelite_type != sad::TIME_ROW_SENTINEL_COMBINATION) {
                    geo = m_geo;
                } else {
                    geo = m_time_row_geo[0];
                }
                QPointF pixel = geoToPixel(latLon.x(), latLon.y(), geo);
                if (pixel.x() < 0 && pixel.y() < 0)
                    return;  // конвертация не удалась
                samplePointOnSceneChangedEvent(pixel);
                centerSceneOnCrossSquare();
            });
}

void MainWindowSatelliteComparator::setExternalSampleFromClipboard() {
    /*QJsonObject jo_source;
    QJsonObject satellites;
    QJsonArray responses;
    jsn::getJsonObjectFromFile(":/res/sd.json", jo_source);
    QString str = QJsonDocument(jo_source).toJson(QJsonDocument::Indented);
    qDebug() << "load external spectr from clipboard...." <<
jo_source.keys(); satellites = jo_source["satellites"].toObject();

    QVector<QVector<double>> result;
    const QString path =
        QApplication::applicationDirPath() + "/S2Cresponses.txt";
    QFileInfo fi(path);
    qDebug() << "is sentinel 2C exists: " << fi.isFile();
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "sentinel 2c file open error......";
        return;
    };

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split('\t');
        if (parts.size() < 14) continue;
        qDebug() << "--->" << parts;
        parts.removeFirst();  // игнорируем первый столбец

        QVector<double> row;
        row.reserve(parts.size());
        QJsonArray jarr;
        for (const QString &s : qAsConst(parts)) {
            bool ok = false;
            double value = s.toDouble(&ok);
            if (!ok) continue;  // или return result; если нужно строгое
чтение row.push_back(value); jarr.append(value);
        }

        if (!row.isEmpty()) {
            result.push_back(row);
            responses.append(jarr);
        }
    }

    qDebug() << "result size: " << result.size();
    QJsonObject temp;
    temp["responses"] = responses;
    temp["alias"] = "SENTINEL 2C";
    QJsonArray central_waves = {443, 490, 560, 665,  705,  740, 783,
                                842, 865, 945, 1375, 1610, 2190};

    temp["central_waves"] = central_waves;
    satellites["sentinel2C"] = temp;
    jo_source["satellites"] = satellites;
    jsn::saveJsonObjectToFile(QApplication::applicationDirPath() +
"/test.json", jo_source);*/
    // Очищаем векторы перед записью новых данных
    QVector<double> waves;
    QVector<double> valuesw;

    // 1. Забираем текст из системного буфера обмена
    QClipboard *clipboard = QApplication::clipboard();
    QString clipboardText = clipboard->text();

    if (clipboardText.isEmpty()) {
        qWarning() << "Буфер обмена пуст!";
        return;
    }

    // 2. Создаем поток для построчного чтения текста
    QTextStream stream(&clipboardText, QIODevice::ReadOnly);

    // 3. Игнорируем первые 3 строки заголовка
    /*for (int i = 0; i < 3; ++i) {
        if (stream.atEnd()) {
            qWarning() << "Ошибка: В буфере обмена слишком мало строк!";
            return;
        }
        stream.readLine();
    }*/

    // 4. Принудительно используем точку '.' как разделитель дроби
    // (C-локаль)
    QLocale cLocale(QLocale::C);
    bool okWave = false;
    bool okVal = false;

    // Регулярное выражение для разделения по любым пробелам/табуляциям
    QRegularExpression spacesOrTabs("\\s+");

    // 5. Читаем оставшиеся строки с данными
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;  // Пропускаем пустые строки

        // Разбиваем строку на элементы
        QStringList tokens = line.split(spacesOrTabs);

        // Проверяем, что в строке есть как минимум два значения
        if (tokens.size() >= 2) {
            double wave = cLocale.toDouble(tokens.at(0), &okWave);
            double val = cLocale.toDouble(tokens.at(1), &okVal);

            // Если оба числа успешно распарсились, добавляем их в
            // соответствующие векторы
            if (okWave && okVal) {
                waves.append(wave);
                valuesw.append(val);
            } else {
                qWarning() << "Не удалось распознать числа в строке:" << line;
            }
        }
    }

    qDebug() << "Считано точек:" << waves.size();
    m_is_bekas = true;
    processBekasDataForComparing(waves, valuesw);
    QString bekas_sample;
    for (int i = 0; i < m_bekas_sample.size(); ++i) {
        bekas_sample.append(QString::number(m_bekas_sample[i]));
        bekas_sample.append("\n");
    }
    clipboard->setText(bekas_sample);
}

void MainWindowSatelliteComparator::onStretchParamsChanged() {
    double lowPct = ui->widget_image_saturation_light_corrector->getLowPct();
    double highPct = ui->widget_image_saturation_light_corrector->getHighPct();
    double gamma = ui->widget_image_saturation_light_corrector->getGamma();

    // --- Обновление основного снимка ---
    if (m_current_r) {
        QImage imgNew = buildRgbPercentile(
            m_current_r, m_current_g, m_current_b, m_current_w, m_current_h,
            m_current_mask, lowPct, highPct, gamma);
        if (!imgNew.isNull()) {
            m_satellite_image = imgNew;
            ui->widget_image_saturation_light_corrector
                ->setDefaultSatLightValues();
            auto pixmap = QPixmap::fromImage(m_satellite_image);
            m_scene->removeItem(m_image_item);
            delete m_image_item;
            m_image_item = new QGraphicsPixmapItem(pixmap);
            m_image_item->setCursor(Qt::CrossCursor);
            m_image_item->setZValue(Z_INDEX_BASE_IMAGE);
            m_scene->addItem(m_image_item);
            m_scene->update();
        }
    }

    // --- Обновление снимков временного ряда ---
    if (!m_viewers.empty() && !m_time_row.empty()) {
        auto imgs = get_cropedImages_for_time_row(m_time_row, m_satelite_type);
        for (int i = 0; i < imgs.size() && i < m_viewers.size(); ++i) {
            m_viewers[i]->setImage(QPixmap::fromImage(imgs[i]));
        }
    }
}

void MainWindowSatelliteComparator::createImageWithAtmCorrecton() {
    // Запускаем весь процесс асинхронно в фоновом потоке
    if (m_sentinel_data.empty()) {
        qDebug() << "Create Image with Atm correction Failed.....Because "
                    "sentinel data EMPTY";
        return;
    };
    QtConcurrent::run([this]() {
        const int x = m_sentinel_data[0].width;
        const int y = m_sentinel_data[0].height;
        double BLUE_band = 0.0;
        double GREEN_band = 0.0;
        double RED_band = 0.0;

        QImage img(300, 300, QImage::Format_RGB32);

        for (int i = 1000; i < 1300; ++i) {
            for (int j = 1000; j < 1300; ++j) {
                auto ksy = getSentinelKsy(j, i);
                ksy.second.resize(10);
                m_ac.getAlbedoBySpeya(ksy.second);
                BLUE_band = ksy.second[1];
                GREEN_band = ksy.second[2];
                RED_band = ksy.second[3];

                // Перевод нормированных 0..1 в RGB 0..255
                int r = static_cast<int>(RED_band * 255);
                int g = static_cast<int>(GREEN_band * 255);
                int b = static_cast<int>(BLUE_band * 255);

                // Ограничиваем значения в диапазон 0..255 на случай выхода за
                // 0..1
                r = qBound(0, r, 255);
                g = qBound(0, g, 255);
                b = qBound(0, b, 255);

                // Записываем пиксель в локальные координаты QImage (от 0 до
                // 999)
                img.setPixel(j - 1000, i - 1000, qRgb(r, g, b));
                qDebug() << "pixel: " << i << " - " << j;
            }
        }

        // Путь для сохранения во временную директорию ОС
        QString filePath = QDir::tempPath() + "/sentinel_output.png";

        // Сохраняем картинку на диск
        if (img.save(filePath)) {
            // Открываем файл программой по умолчанию в ОС
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    });
}

void MainWindowSatelliteComparator::calculateSen2CorCATIaccuracy() {
    qDebug() << m_sentinel_data.size();
    qDebug() << m_sen2cor_data.size();
    if (m_sen2cor_data.size() < 10 || m_sentinel_data.size() < 10) {
        qDebug() << "Size is less than required";
        return;
    }

    // Инициализация и запуск таймера
    QElapsedTimer timer;
    timer.start();

    // Возвращаем оригинальное получение размеров из нулевого элемента
    const int width = m_sentinel_data[0].width;
    const int height = m_sentinel_data[0].height;
    qDebug() << "width: " << width;
    qDebug() << "height: " << height;

    const quint64 total_pixels = static_cast<quint64>(width) * height;
    qDebug() << "common_pixels_number: " << total_pixels;

    // Шаг прогресса по строкам (вывод в консоль только 60 раз за весь расчет)
    const int progress_step_y = qMax(1, height / 60);

    quint32 vegetation_counter = 0;
    quint32 not_vegetation_counter = 0;
    quint32 water_counter = 0;
    quint32 unclassified_counter = 0;
    quint32 general_counter = 0;

    // Быстрый массив на стеке для накопления квадратов разностей
    double general_RMSEs[10] = {0.0};

    // Защита на случай, если указатель на маску нулевой
    if (!mask_for_sen2cor_data) {
        qDebug() << "Error: mask_for_sen2cor_data is null!";
        return;
    }

    for (int y = 0; y < height; ++y) {
        // Проверка прогресса во внешнем цикле
        if (y % progress_step_y == 0) {
            int percentage = (y * 100) / height;
            qDebug() << "Progress:" << percentage << "% (row" << y << "/"
                     << height << ")"
                     << "| Elapsed time:" << timer.elapsed() << "ms";
        }

        const quint64 row_offset = static_cast<quint64>(y) * width;

        for (int x = 0; x < width; ++x) {
            const quint64 current_pixel_index = row_offset + x;

            // Прямой доступ к сырому указателю uint16_t* без лишних методов
            const auto class_value = mask_for_sen2cor_data[current_pixel_index];

            // Быстрая фильтрация классов
            if (class_value == 4) {
                ++vegetation_counter;
            } else if (class_value == 5) {
                ++not_vegetation_counter;
            } else if (class_value == 6) {
                ++water_counter;
            } else if (class_value == 7) {
                ++unclassified_counter;
            } else {
                continue;
            }

            ++general_counter;

            // Вызовы функций пиксельной обработки
            const auto speya = getSentinelSpeyaValues(x, y);
            const auto sen2cor_ksy = getSen2CorKsy(x, y);
            const auto cati = m_ac.calculateAlbedo(speya);

            if (cati.size() < 10) {
                continue;
            }

            // Быстрый расчет во внутреннем цикле
            for (int i = 0; i < 10; ++i) {
                const double dif = sen2cor_ksy[i] - cati[i];
                general_RMSEs[i] += dif * dif;
            }
        }
    }

    // Фиксация финального времени
    const qint64 total_elapsed_ms = timer.elapsed();

    qDebug() << "Progress: 100 % (" << total_pixels << "/" << total_pixels
             << ")"
             << "| Total time:" << total_elapsed_ms << "ms ("
             << (static_cast<double>(total_elapsed_ms) / 1000.0) << "sec)";
    qDebug() << "vegetation pixels number: " << vegetation_counter;
    qDebug() << "general counter: " << general_counter;

    if (general_counter == 0) {
        qDebug() << "No pixels were processed for RMSE calculation.";
        return;
    }

    const double prefix = 1.0 / general_counter;
    for (int i = 0; i < 10; ++i) {
        QString result = "general RMSE %1 channel: %2";
        qDebug() << result.arg(i + 1).arg(std::sqrt(prefix * general_RMSEs[i]));
    }
}

void MainWindowSatelliteComparator::showRgbImage(const uint16_t *r,
                                                 const uint16_t *g,
                                                 const uint16_t *b, int width,
                                                 int height, double lowPct,
                                                 double highPct, double gamma,
                                                 const uint16_t *cloudMask) {
    ProgressInformator progress_info(ui->graphicsView_satellite_image,
                                     satc::message_changing_bands);
    progress_info.show();
    QApplication::processEvents();

    // Сохраняем сырые указатели для возможного пересчёта при изменении
    // параметров
    m_current_r = r;
    m_current_g = g;
    m_current_b = b;
    m_current_w = width;
    m_current_h = height;
    m_current_mask = cloudMask;

    QImage imgNew = buildRgbPercentile(r, g, b, width, height, cloudMask,
                                       lowPct, highPct, gamma);

    if (imgNew.isNull()) return;

    if (m_image_item) {
        m_scene->removeItem(m_image_item);
        delete m_image_item;
    }
    m_satellite_image = imgNew;
    auto pixmap = QPixmap::fromImage(m_satellite_image);
    m_image_item = new QGraphicsPixmapItem(pixmap);
    m_image_item->setCursor(Qt::CrossCursor);
    m_image_item->setZValue(Z_INDEX_BASE_IMAGE);
    m_scene->addItem(m_image_item);
    m_scene->setSceneRect(pixmap.rect());
    ui->graphicsView_satellite_image->centerOn(m_image_item);
    updateImage();
}

void MainWindowSatelliteComparator::create_index_dynamic_maps(
    const QString &roiId) {
    if (m_time_row.empty()) return;

    const int N = m_time_row.size();
    if (m_time_row_dates_unix_time.first.size() != N) {
        qWarning() << "Нет временных меток для временного ряда";
        return;
    }

    const QString ndviLayerId = "TIME_GRADIENT_NDVI_" + roiId;
    const QString ndwiLayerId = "TIME_GRADIENT_NDWI_" + roiId;
    const QString summaryLayerId = "TIME_GRADIENT_AGREGATION_" + roiId;

    // Удаляем предыдущие результаты для этого ROI
    for (const QString &key : {ndviLayerId, ndwiLayerId, summaryLayerId}) {
        if (m_layers_search_result_items.contains(key)) {
            m_layer_gui_list->removeItemList(key);
        }
    }

    QVector<double> julianDays(N);
    const qint64 t0 = m_time_row_dates_unix_time.first[0];
    for (int i = 0; i < N; ++i) {
        julianDays[i] =
            static_cast<double>(m_time_row_dates_unix_time.first[i] - t0) /
            86400.0;
    }

    int xSize = INT_MAX;
    int ySize = INT_MAX;
    for (int i = 0; i < N; ++i) {
        xSize = qMin(xSize, m_time_row[i][0].width);
        ySize = qMin(ySize, m_time_row[i][0].height);
    }

    auto roi_item = ui->graphicsView_satellite_image->getPolygonById(roiId);
    if (!roi_item) return;

    QPolygonF polygon = roi_item->mapToScene(roi_item->polygon());
    QRectF bbox = roi_item->mapToScene(roi_item->boundingRect()).boundingRect();

    QVector<QPointF> insidePoints;
    for (int x = static_cast<int>(bbox.left());
         x <= static_cast<int>(bbox.right()); ++x) {
        for (int y = static_cast<int>(bbox.top());
             y <= static_cast<int>(bbox.bottom()); ++y) {
            if (polygon.containsPoint(QPointF(x, y), Qt::OddEvenFill)) {
                insidePoints.append(QPointF(x, y));
            }
        }
    }

    if (insidePoints.isEmpty()) return;
    qDebug() << "=-=-=-=-=-=-=-";
    GradientMaskResult ndviMask =
        buildGradientClassMap(insidePoints, julianDays, xSize, ySize, 0);
    GradientMaskResult ndwiMask =
        buildGradientClassMap(insidePoints, julianDays, xSize, ySize, 1);

    if (!ndviMask.isValid() && !ndwiMask.isValid()) {
        qWarning() << "Не удалось построить ни NDVI, ни NDWI маску";
        return;
    }

    GradientMaskResult summaryMask =
        buildCombinedGradientClassMap(ndviMask, ndwiMask);

    if (ndviMask.isValid()) {
        if (auto *item = buildGradientMaskItem(ndviMask)) {
            m_scene->addItem(item);
            m_layers_search_result_items.insert(ndviLayerId, item);
            m_layer_gui_list->addItemToList(ndviLayerId,
                                            buildIndexDynamicsLegendTooltip(),
                                            QColor(34, 139, 34), Qt::Unchecked);
        }
    }

    if (ndwiMask.isValid()) {
        if (auto *item = buildGradientMaskItem(ndwiMask)) {
            m_scene->addItem(item);
            m_layers_search_result_items.insert(ndwiLayerId, item);
            m_layer_gui_list->addItemToList(
                ndwiLayerId, buildIndexDynamicsLegendTooltip(),
                QColor(30, 144, 255), Qt::Unchecked);
        }
    }

    if (summaryMask.isValid()) {
        if (auto *item = buildGradientMaskItem(summaryMask)) {
            m_scene->addItem(item);
            m_layers_search_result_items.insert(summaryLayerId, item);
            m_layer_gui_list->addItemToList(summaryLayerId,
                                            buildIndexDynamicsLegendTooltip(),
                                            QColor(200, 200, 30));
        }
    }
}

GradientMaskResult MainWindowSatelliteComparator::buildGradientClassMap(
    const QVector<QPointF> &insidePoints, const QVector<double> &julianDays,
    int xSize, int ySize, int indexType) {
    GradientMaskResult result;
    result.xSize = xSize;
    result.ySize = ySize;
    result.classes.fill(-1, xSize * ySize);
    result.gradients.fill(std::numeric_limits<double>::quiet_NaN(),
                          xSize * ySize);
    result.r2.fill(0.0, xSize * ySize);

    const int N = m_time_row.size();
    const bool is_landsat =
        (m_satelite_type == sad::TIME_ROW_LANDSAT_COMBINATION);

    const int red_band = 3;
    const int nir_band = is_landsat ? 4 : 6;
    const int swir_band = is_landsat ? 5 : 9;

    for (const QPointF &pt : insidePoints) {
        int px = static_cast<int>(pt.x());
        int py = static_cast<int>(pt.y());
        if (px < 0 || px >= xSize || py < 0 || py >= ySize) continue;

        double latitude = 0.0, longitude = 0.0;
        getGeoCoordinates(px, py, m_time_row_geo[0], latitude, longitude,
                          false);

        QVector<QPointF> pts(N);
        pts[0] = QPointF(px, py);

        bool anyBad = false;
        for (int i = 1; i < N; ++i) {
            pts[i] = geoToPixel(latitude, longitude, m_time_row_geo[i]);
            int bx = static_cast<int>(pts[i].x());
            int by = static_cast<int>(pts[i].y());
            if (bx < 0 || bx >= xSize || by < 0 || by >= ySize) {
                anyBad = true;
                break;
            }
        }
        if (anyBad) continue;

        if (!isDataCloudShadow_OK(pts)) {
            continue;
        }

        QVector<double> indexSeries;
        QVector<double> xValid;

        for (int i = 0; i < N; ++i) {
            int bx = static_cast<int>(pts[i].x());
            int by = static_cast<int>(pts[i].y());

            auto getValue = [&](int bandIdx) -> double {
                auto &bd = m_time_row[i][bandIdx];
                uint16_t raw = bd.data[by * bd.width + bx];
                return is_landsat
                           ? (bd.reflectance_mult * raw + bd.reflectance_add)
                           : (raw / 10000.0);
            };

            double red = getValue(red_band);
            double nir = getValue(nir_band);
            double swir = getValue(swir_band);

            if (red <= 0 || nir <= 0 || swir <= 0) continue;
            if (red > 1 || nir > 1 || swir > 1) continue;

            double val = (indexType == 0) ? sam::calculateNDVI(nir, red)
                                          : sam::calculateSWVI(nir, swir);

            indexSeries.push_back(val);
            xValid.push_back(julianDays[i]);
        }

        if (xValid.size() < 3) continue;

        auto fit = fitLinear(xValid, indexSeries);
        if (!fit.valid) continue;

        int dpClass = (indexType == 0) ? classifyByNdviGradient(fit.G, fit.R2)
                                       : classifyByNdwiGradient(fit.G, fit.R2);

        if (dpClass < 0) continue;

        const int idx = py * xSize + px;
        result.classes[idx] = dpClass;
        result.gradients[idx] = fit.G;
        result.r2[idx] = fit.R2;
    }

    return result;
}

GradientMaskResult MainWindowSatelliteComparator::buildCombinedGradientClassMap(
    const GradientMaskResult &ndviMask, const GradientMaskResult &ndwiMask) {
    GradientMaskResult result;
    if (!ndviMask.isValid() && !ndwiMask.isValid()) {
        return result;
    }

    const GradientMaskResult *base = ndviMask.isValid() ? &ndviMask : &ndwiMask;
    result.xSize = base->xSize;
    result.ySize = base->ySize;
    result.classes.fill(-1, result.xSize * result.ySize);
    result.gradients.fill(std::numeric_limits<double>::quiet_NaN(),
                          result.xSize * result.ySize);
    result.r2.fill(0.0, result.xSize * result.ySize);

    for (int i = 0; i < result.classes.size(); ++i) {
        const int cNdvi = ndviMask.isValid() ? ndviMask.classes[i] : -1;
        const int cNdwi = ndwiMask.isValid() ? ndwiMask.classes[i] : -1;

        const double gNdvi = ndviMask.isValid() ? ndviMask.gradients[i] : 0.0;
        const double gNdwi = ndwiMask.isValid() ? ndwiMask.gradients[i] : 0.0;

        const double r2Ndvi = ndviMask.isValid() ? ndviMask.r2[i] : 0.0;
        const double r2Ndwi = ndwiMask.isValid() ? ndwiMask.r2[i] : 0.0;

        const int combinedClass =
            combineGradientClasses(cNdvi, gNdvi, r2Ndvi, cNdwi, gNdwi, r2Ndwi);

        result.classes[i] = combinedClass;

        // дозополняем поля структуры на основе данных с NDVI и NDWI
        if (combinedClass >= 0) {
            if (cNdvi >= 0 && cNdwi >= 0) {
                result.gradients[i] = 0.5 * (gNdvi + gNdwi);
                result.r2[i] = qMax(r2Ndvi, r2Ndwi);
            } else if (cNdvi >= 0) {
                result.gradients[i] = gNdvi;
                result.r2[i] = r2Ndvi;
            } else if (cNdwi >= 0) {
                result.gradients[i] = gNdwi;
                result.r2[i] = r2Ndwi;
            }
        }
    }

    return result;
}

QGraphicsPixmapItem *MainWindowSatelliteComparator::buildGradientMaskItem(
    const GradientMaskResult &mask, const QColor &layerColorHint) {
    Q_UNUSED(layerColorHint);

    if (!mask.isValid()) return nullptr;

    auto *new_layer = new uchar[mask.xSize * mask.ySize * 4];
    std::memset(new_layer, 0, mask.xSize * mask.ySize * 4);

    bool hasAnyPixel = false;

    for (int y = 0; y < mask.ySize; ++y) {
        for (int x = 0; x < mask.xSize; ++x) {
            const int cls = mask.classes[y * mask.xSize + x];
            if (cls < 0) continue;

            QColor color = dpClassColor(cls);
            const int offset = (y * mask.xSize + x) * 4;
            new_layer[offset] = color.red();
            new_layer[offset + 1] = color.green();
            new_layer[offset + 2] = color.blue();
            new_layer[offset + 3] = 254;
            hasAnyPixel = true;
        }
    }

    if (!hasAnyPixel) {
        delete[] new_layer;
        return nullptr;
    }

    auto cleanup = [](void *info) { delete[] static_cast<uchar *>(info); };
    QImage img(new_layer, mask.xSize, mask.ySize, mask.xSize * 4,
               QImage::Format_RGBA8888, cleanup, new_layer);

    auto *item = new QGraphicsPixmapItem(QPixmap::fromImage(img));
    item->setZValue(ui->graphicsView_satellite_image->getMaxZValue(m_scene));
    return item;
}
