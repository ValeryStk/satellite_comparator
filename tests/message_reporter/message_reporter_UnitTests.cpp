#include "message_reporter_UnitTests.h"

#include <QDebug>

#include "message_reporter.cpp"

namespace {}  // end namespace

message_reporter_UnitTests::message_reporter_UnitTests() {}

void message_reporter_UnitTests::initTestCase() {
    // Инициализация перед запуском всех тестов
}

void message_reporter_UnitTests::cleanupTestCase() {
    // Очистка после выполнения всех тестов
}

void message_reporter_UnitTests::init() {
    // Инициализация перед каждым тестом
}

void message_reporter_UnitTests::cleanup() {
    // Очистка после каждого теста
}

void message_reporter_UnitTests::showMessages() {
    uts::showErrorMessage("Ошибка разбора XML",
                          "Заголовочный файл XML повреждён");
    uts::showErrorMessage("Ошибка разбора TXT",
                          "Заголовочный файл TXT повреждён");
    uts::showErrorMessage("Ошибка разбора JSON",
                          "Заголовочный файл JSON повреждён");
}

QTEST_MAIN(message_reporter_UnitTests)
