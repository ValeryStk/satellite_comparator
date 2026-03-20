#include "atm_correction_UnitTests.h"

#include <QDebug>

#include "calculation_solver.h"
#include "json_utils.h"

namespace {}  // end namespace

atm_correction_UnitTests::atm_correction_UnitTests() {}

void atm_correction_UnitTests::initTestCase() {
    // Инициализация перед запуском всех тестов
}

void atm_correction_UnitTests::cleanupTestCase() {
    // Очистка после выполнения всех тестов
}

void atm_correction_UnitTests::init() {
    // Инициализация перед каждым тестом
}

void atm_correction_UnitTests::cleanup() {
    // Очистка после каждого теста
}

void atm_correction_UnitTests::loadSattelitesData() {
    qDebug() << "atm correction test...";
    calculation_solver cs;
    // cs.updateCurrentSatellite("sentinel2a-20m");
    /*QJsonArray jar;
    QJsonArray out_jar;
    jsn::getJsonArrayFromFile("sdb.json", jar);
    for (int i = 0; i < jar.size(); ++i) {
        QJsonObject obj = jar[i].toObject();
        QJsonObject out_obj;

        out_obj["h2o"] = obj["h2o"].toDouble();
        out_obj["o2"] = obj["o2"].toDouble();
        out_obj["o3"] = obj["o3"].toDouble();
        out_obj["sun"] = obj["sun"].toDouble();
        out_obj["wavelength"] = obj["wavelength"].toDouble() * 1000;
        out_obj["sentinel2a"] = obj["sentinel2a-20m"].toArray();
        out_obj["sentinel2b"] = obj["sentinel2b-20m"].toArray();
        out_jar.append(out_obj);
    }
    jsn::saveJsonArrayToFile("test.json", out_jar, QJsonDocument::Indented);*/
}

void atm_correction_UnitTests::calculateCosSunZenitAngle() {}

QTEST_MAIN(atm_correction_UnitTests)
