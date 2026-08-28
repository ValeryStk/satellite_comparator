#include "calculation_solver.h"

#include <QFuture>
#include <QFutureWatcher>  // Опционально, для слотов finished()
#include <QtConcurrent/QtConcurrent>

#include "atm_correction.cpp"

calculation_solver::calculation_solver(QVector<double> initial_values) {
    m_initial_values = initial_values;
    ::loadAllLists();
}

std::vector<double> calculation_solver::getLambdaList() const {
    return lambda_list;
}

std::vector<std::vector<double>> calculation_solver::getResponsesList() {
    return S_lambda_lists;
}

std::vector<double> calculation_solver::get_h2o() { return T_H2O_list; }

std::vector<double> calculation_solver::get_o3() { return T_O3_list; }

std::vector<double> calculation_solver::get_a_H2O() {
    return loadDoublesFromFile(":/atm_params/a_H2O.txt");
}

std::vector<double> calculation_solver::get_b_H2O() {
    return loadDoublesFromFile(":/atm_params/b_H2O.txt");
}

double calculation_solver::get_mH2O(double B9, double B8A) {
    return compute_mH2O(B9, B8A);
}

double calculation_solver::get_TO3(int band_index, int X) {
    Q_ASSERT(band_index < NUMBER_OF_CHANNELS && band_index >= 0);
    if (band_index > NUMBER_OF_CHANNELS - 1) return -1;
    return compute_TO3(dobson_TiO[band_index], dobson_alfa[band_index], X);
}

void calculation_solver::updateCurrentSatellite(QString sat_name) {
    lss::updateSatelliteResponses(sat_name);
}

QVector<double> calculation_solver::calculateAlbedo(
    QVector<double> speya_values, int classNum) {
    return QVector<double>::fromStdVector(
        ::calculateAlbedoFinal(speya_values, classNum));
}

void calculation_solver::start_solve_dark_pixels_async(
    const QString &satellite_name, const QVector<double> &dark_pixels) {
    qDebug()
        << "----------START ASYNC SOLVE DARK PIXEL------------------------";
    if (dark_pixels.size() != 10) {
        qDebug() << "Wrong chennels number -> " << dark_pixels.size()
                 << "Must be 10";
        return;
    }

    // Lambda захватывает this и аргументы по ссылке
    auto task = [this, satellite_name, dark_pixels]() {
        qDebug() << "Solve dark pixel for satellite: " << satellite_name
                 << " (in thread:" << QThread::currentThread() << ")";
        auto result =
            lss::optimize(satellite_name, dark_pixels, m_initial_values);
        emit darkpixels_calculation_finished(
            result);  // Безопасно из любого потока
    };

    QFuture<void> future = QtConcurrent::run(task);

    // Опционально: QFutureWatcher для обработки завершения
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    watcher->setFuture(future);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        qDebug() << "Dark pixels calculation finished";
        watcher->deleteLater();
    });
}

std::vector<double> calculation_solver::loadDoublesFromFile(
    const QString &filePath) {
    std::vector<double> values;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return values;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        bool ok = false;
        const double value = line.toDouble(&ok);
        if (ok) values.push_back(value);
    }

    return values;
}

void calculation_solver::setH2O(QVector<double> new_h20_list) {
    // Добавить поправку по каналам чувствительности 8A и 9
    T_H2O_list = new_h20_list.toStdVector();
}

void calculation_solver::setSunZenitAngle(double angle) {
    lss::setSunZenitAngle(angle);
}

void calculation_solver::setCaptruretZenitAngle(double angle) {
    lss::setCaptureZenitAngle(angle);
}

void calculation_solver::setFiAngle(double angleSA, double angleCA) {
    lss::setFiAngle(angleSA, angleCA);
}

void calculation_solver::computeGamma() { lss::compute_gamma(mu, mu_0, fi); }

void calculation_solver::solve_dark_pixels(const QString &satellite_name,
                                           const QVector<double> &dark_pixels) {
    qDebug() << "----------SOLVE DARK PIXEL------------------------";
    if (dark_pixels.size() < 10) return;
    qDebug() << "Solve pixel for satellite: " << satellite_name;
    auto result = lss::optimize(satellite_name, dark_pixels, m_initial_values);
    emit darkpixels_calculation_finished(result);
}

void calculation_solver::setInitial_values(
    const QVector<double> &initial_values) {
    qDebug() << "initial_values: " << initial_values;
    m_initial_values = initial_values;
}
