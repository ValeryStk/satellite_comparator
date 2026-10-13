#include "progress_informator.h"

#include <QApplication>
#include <QScreen>

#include "QGuiApplication"

namespace msg {

const char kLoadLandSatTimeRowTask[] =
    "Пожалуйста, подождите...\n"
    "Загружается временной ряд со спутника Landsat";

const char kLoadSentinelTimeRowTask[] =
    "Пожалуйста, подождите...\n"
    "Загружается временной ряд со спутника Sentinel";

const char kLoadSingleSentinelTask[] =
    "Пожалуйста, подождите...\n"
    "Загружается базовый снимок со спутника Sentinel";

const char kLoadSentinel_Level_1_no_atm_correction[] =
    "Пожалуйста, подождите...\n"
    "Загружается снимок со спутника Sentinel\n без атмосферной коррекции";

const char kLoadSingleLandsatTask[] =
    "Пожалуйста, подождите...\n"
    "Загружается базовый снимок со спутника Landsat";

const char kLoadSentinelWithAtmCorrection[] =
    "Пожалуйста, подождите...\n"
    "Загружаются данные Sentinel с атмосферной коррекцией...";

const char kFindProcessIsRunning[] =
    "Пожалуйста, подождите...\n"
    "Выполняется поиск областей с использованием: %1...";

const char kCalculateSen2Cor_vs_CATI_metrics[] =
    "Пожалуйста подождите...\nВыполняется расчёт метрик  для оценки схожести\n "
    "Sen2Cor vs CATI";

const char kFindBasePixelAsMoreLinear[] =
    "Пожалуйста подождите...\nВыполняется поиск базового пикселя\n(линейный "
    "метод)";

const char kSaveCATIasGeoTiff[] =
    "Пожалуйста подождите...\nВыполняется экспорт CATI в Geo TIFF...";

const char kSaveBaseSentinelAs20mGeoTiff[] =
    "Пожалуйста подождите...\nВыполняется экспорт данных Sentinel в Geo "
    "TIFF...";

}  // namespace msg

ProgressInformator::ProgressInformator(QWidget *parent, const QString &text) {
    this->setParent(parent);
    this->setStyleSheet("background-color: white; border: 1px solid black;");
    QFont font("Arial", 16, QFont::Bold);
    this->setFont(font);
    this->setText(text);
    this->adjustSize();
    int x = (parent->width() - this->width()) / 2;
    int y = (parent->height() - this->height()) / 2;
    this->move(x, y);
    show();
    QApplication::processEvents();
}

ProgressInformator::~ProgressInformator() {
    this->hide();
    this->setParent(nullptr);
}
