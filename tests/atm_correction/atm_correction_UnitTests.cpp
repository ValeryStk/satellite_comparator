#include "atm_correction_UnitTests.h"

#include <QDebug>

#include "calculation_solver.h"
#include "davis.h"
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
    // qDebug() << "atm correction test...";
    // calculation_solver cs;
    //  cs.updateCurrentSatellite("sentinel2a-20m");
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
    QJsonArray jar;
    jsn::getJsonArrayFromFile(
        QCoreApplication::applicationDirPath() + "/sentinel2B_responses.json",
        jar);
    dv::holdOn();
    QVector<double> full_waverange(601);
    std::iota(full_waverange.begin(), full_waverange.end(), 400);
    for (int j = 0; j < jar.size(); ++j) {
        QVector<double> full_values(601, 0);
        auto arr1 = jar[j].toObject()["spectral_response"].toArray();
        int wave_offset = jar[j].toObject()["wavelength_MIN_nm"].toInt();
        QString name = jar[j].toObject()["physicalBand"].toString();

        for (int i = 0; i < arr1.size(); ++i) {
            if (wave_offset > 1000) break;
            int index = wave_offset - 400 + i;
            full_values[index] = arr1[i].toDouble();
        }
        dv::show(full_waverange, full_values, name.toStdString());
    }
    dv::holdOff();
}

void atm_correction_UnitTests::calculateCosSunZenitAngle() {}

QTEST_MAIN(atm_correction_UnitTests)
