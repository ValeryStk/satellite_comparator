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
    QJsonArray atm_params_jar;
    jsn::getJsonArrayFromFile(
        ":/responses/sentinel2B/sentinel2B_responses.json", sentinel2B_jar);
    jsn::getJsonArrayFromFile(
        ":/responses/sentinel2A/sentinel2A_responses.json", sentinel2A_jar);
    jsn::getJsonArrayFromFile(":/atm_params/atm_params.json", atm_params_jar);
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

void atm_correction_UnitTests::calculateCosSunZenitAngle() {
    /*setCellValue("X", 300);
    setCellValue("q", 2);
    setCellValue("p", 1.25);
    setCellValue("Tau_m0", 0.098, 3);
    setCellValue("Tau_a0", 0.2);
    setCellValue("Beta", 2);
    setCellValue("Tau_e", 0.04);
    setCellValue("g_a", 0.6);
    setCellValue("p_1", 0.05);
    setCellValue("p_2", 0.15);*/
    calculation_solver cs({300, 2, 1.25, 3, 0.2, 2, 0.04, 0.6, 0.05, 0.15});
    cs.get_mH2O(10.7995, 53.7519);
    cs.setSunZenitAngle(45);
    cs.setCaptruretZenitAngle(45);
    cs.setFiAngle(45, 78);
    cs.computeGamma();
    cs.solve_dark_pixels("sentinel 2A",
                         {87.94741, 77.81722, 67.03251, 61.8863, 56.85879,
                          56.87943, 57.46667, 50.95204, 53.7519, 10.7995});
}

QTEST_MAIN(atm_correction_UnitTests)
