#include "SamUnitTests.h"

#include <QDebug>

#include "health_ranges.h"
#include "sam.h"

namespace {}  // end namespace

SamUnitTests::SamUnitTests() {}

void SamUnitTests::initTestCase() {
    // Инициализация перед запуском всех тестов
}

void SamUnitTests::cleanupTestCase() {
    // Очистка после выполнения всех тестов
}

void SamUnitTests::init() {
    // Инициализация перед каждым тестом
}

void SamUnitTests::cleanup() {
    // Очистка после каждого теста
}

void SamUnitTests::samMetricsTest() {
    double result;
    sam::ProcessingResult pr;
    QVector<double> v1 = {};
    QVector<double> v2 = {};
    pr = sam::calculateEuclideanDistance(v1, v2, result);
    QString message = QString::fromStdString(pr.message);
    QCOMPARE(pr.status, sam::STATUS_CODE::ONE_OF_THE_VECTORS_ARE_EMPTY);
    qDebug() << message << result;
    v1 = {0.8, 0.6, 0.2};
    v2 = {0.8, 0.55, 0.25};
    pr = sam::calculateEuclideanDistance(v1, v2, result);
    QCOMPARE(pr.status, sam::STATUS_CODE::OK);
    qDebug() << QString::fromStdString(pr.message) << result;
}

void SamUnitTests::samTreeStatesTest() {
    // Создаем объект для оценки состояния растений
    PlantHealth health;

    // Тестовые значения индекса
    double testValues[] = {1.8, 1.3, 1.1, 0.9, 0.7, 0.3, -0.5};

    qDebug() << "=== Оценка состояния растений по вегетационному индексу ===";

    for (double value : testValues) {
        int healthClass = health.getHealthClass(value);

        qDebug() << "\nИндекс = " << value;
        qDebug() << "  Класс: " << healthClass;
        qDebug() << "  Состояние: "
                 << QString::fromStdString(health.getClassName(healthClass));
        qDebug() << "  Цвет (HEX): "
                 << QString::fromStdString(health.getClassColor(healthClass));
        qDebug() << "  Описание: "
                 << QString::fromStdString(health.getDescription(healthClass));
    }
}

QTEST_MAIN(SamUnitTests)
