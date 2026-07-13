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
                // qDebug() << "offset_index: " << index;
            };
            dv::show(full_waverange, full_values,
                     name.toStdString() + " (" +
                         QString::number(central_wave).toStdString() + ")");
        }
        dv::holdOff(cfg);
    }
}

void atm_correction_UnitTests::calculateFixedMathPixel() {
    // clang-format off
                         // X   q   p    tau_m_0  tau_a_0  beta tau_e  g_a   ro_1   ro_2
    calculation_solver cs({300, 2, 1.25, 0.098,    0.2,     2,   0.04, 0.6,  0.05, 0.15});
    // clang-format off
    auto ln_m_H2O = cs.get_mH2O(10.7995, 53.7519);
    cs.setSunZenitAngle(31);
    cs.setCaptruretZenitAngle(9.923);
    cs.setFiAngle(163.981, 290.122);
    cs.computeGamma();

    auto a = cs.get_a_H2O();
    auto b = cs.get_b_H2O();
    auto w = cs.getLambdaList();
    Q_ASSERT(a.size() == b.size() == w.size());
    // qDebug() << a.size() << b.size() << w.size();
    QVector<double> T_H2O;
    QVector<double> T_O3;
    for (size_t i = 0; i < w.size(); ++i) {
        T_H2O.append(a[i] * ln_m_H2O + b[i]);
    }
    cs.setH2O(T_H2O);

    cs.solve_dark_pixels("sentinel 2A",
                         {87.94741, 77.81722, 67.03251, 61.8863, 56.85879,
                          56.87943, 57.46667, 50.95204, 53.7519, 10.7995});
}

QTEST_MAIN(atm_correction_UnitTests)
