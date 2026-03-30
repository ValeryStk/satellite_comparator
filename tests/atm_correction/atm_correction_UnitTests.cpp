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
    //  calculation_solver cs;
    //   cs.updateCurrentSatellite("sentinel2a-20m");
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
    QJsonArray sentinel2B_jar;
    QJsonArray sentinel2A_jar;
    jsn::getJsonArrayFromFile(
        ":/responses/sentinel2B/sentinel2B_responses.json", sentinel2B_jar);
    jsn::getJsonArrayFromFile(
        ":/responses/sentinel2A/sentinel2A_responses.json", sentinel2A_jar);
    QVector<QJsonArray> jarrs = {sentinel2A_jar, sentinel2B_jar};

    QVector<double> full_waverange(601);
    std::iota(full_waverange.begin(), full_waverange.end(), 400);
    dv::Config cfg;

    for (int s = 0; s < jarrs.size(); ++s) {
        dv::holdOn();
        if (s == 0)
            cfg.chart.title = "sentinel2A";
        else
            cfg.chart.title = "sentinel2B";
        for (int j = 0; j < jarrs[s].size(); ++j) {
            QVector<double> full_values(601, 0);
            auto arr1 = jarrs[s][j].toObject()["spectral_response"].toArray();
            int wave_offset =
                jarrs[s][j].toObject()["wavelength_MIN_nm"].toInt();  //
            double central_wave =
                jarrs[s][j].toObject()["wavelength_CENTRAL_nm"].toDouble();
            QString name = jarrs[s][j].toObject()["physicalBand"].toString();
            if (wave_offset > 1000) continue;
            for (int i = 0; i < arr1.size(); ++i) {
                int index = wave_offset - 400 + i;
                if (index > 600) break;
                full_values[index] = arr1[i].toDouble();
                qDebug() << "offset_index: " << index;
            };
            dv::show(full_waverange, full_values,
                     name.toStdString() + " (" +
                         QString::number(central_wave).toStdString() + ")");
        }
        dv::holdOff(cfg);
    }
}

void atm_correction_UnitTests::calculateCosSunZenitAngle() {}

QTEST_MAIN(atm_correction_UnitTests)
