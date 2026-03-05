#include "health_ranges.h"

#include <algorithm>

PlantHealth::PlantHealth() {
    // Инициализация пороговых значений для вегетационного индекса
    // Значения индекса: от 0 до 2+ (чем выше, тем здоровее растение)
    // TODO добавить дипазоны по умолчанию для разных индексов
    // Пороги для классов (верхняя граница класса)
    ranges[1] = 1.5;  // Healthy: > 1.5
    ranges[2] = 1.2;  // Weakened: 1.2 - 1.5
    ranges[3] = 1.0;  // Severely Weakened: 1.0 - 1.2
    ranges[4] = 0.8;  // Dying: 0.8 - 1.0
    ranges[5] = 0.6;  // Fresh Deadwood: 0.6 - 0.8
    ranges[6] = 0.4;  // Old Deadwood: 0.0 - 0.4

    // Названия классов на английском
    classNames[1] = "Healthy";
    classNames[2] = "Weakened";
    classNames[3] = "Severely Weakened";
    classNames[4] = "Dying";
    classNames[5] = "Fresh Deadwood";
    classNames[6] = "Old Deadwood";

    // Цвета классов в формате HEX (по Российскому лесному стандарту)
    classColors[1] = "#008000";  // Зеленый
    classColors[2] = "#90EE90";  // Светло-зеленый
    classColors[3] = "#9ACD32";  // Желто-зеленый
    classColors[4] = "#FFD700";  // Желтый
    classColors[5] = "#CD853F";  // Оранжево-коричневый
    classColors[6] = "#8B4513";  // Темно-коричневый
}

PlantHealth::~PlantHealth() {}

int PlantHealth::getHealthClass(double indexValue) const {
    // Проверка на некорректные значения
    if (indexValue < 0.0) return 6;  // Отрицательные значения - сухостой

    // Определяем класс состояния по значению индекса
    // Идем от здорового к сухостою
    if (indexValue > ranges.at(1)) return 1;  // Healthy
    if (indexValue > ranges.at(2)) return 2;  // Weakened
    if (indexValue > ranges.at(3)) return 3;  // Severely Weakened
    if (indexValue > ranges.at(4)) return 4;  // Dying
    if (indexValue > ranges.at(5)) return 5;  // Fresh Deadwood
    return 6;  // Old Deadwood (включая значения <= ranges[6])
}

double PlantHealth::getThreshold(int healthClass) const {
    auto it = ranges.find(healthClass);
    if (it != ranges.end()) {
        return it->second;
    }
    return -1.0;  // Ошибка: класс не найден
}

std::string PlantHealth::getClassName(int healthClass) const {
    auto it = classNames.find(healthClass);
    if (it != classNames.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string PlantHealth::getClassColor(int healthClass) const {
    auto it = classColors.find(healthClass);
    if (it != classColors.end()) {
        return it->second;
    }
    return "#000000";  // Черный для неизвестного класса
}

void PlantHealth::setThreshold(int healthClass, double value) {
    // Проверяем, что класс существует и значение корректно
    if (isValidClass(healthClass) && value >= 0.0) {
        ranges[healthClass] = value;
    }
}

void PlantHealth::resetToDefaults() {
    // Сбрасываем пороги к значениям по умолчанию
    ranges[1] = 1.5;
    ranges[2] = 1.2;
    ranges[3] = 1.0;
    ranges[4] = 0.8;
    ranges[5] = 0.6;
    ranges[6] = 0.4;
}

std::map<int, double> PlantHealth::getAllThresholds() const { return ranges; }

std::string PlantHealth::getDescription(int healthClass) const {
    switch (healthClass) {
        case 1:
            return "Healthy trees with dense crown and bright green needles";
        case 2:
            return "Weakened trees with sparse crown and light green needles";
        case 3:
            return "Severely weakened trees with very sparse crown and matte "
                   "needles";
        case 4:
            return "Dying trees with yellowing needles and very weak growth";
        case 5:
            return "Fresh deadwood with gray or yellow needles still attached";
        case 6:
            return "Old deadwood with no needles and falling bark";
        default:
            return "Unknown class";
    }
}

bool PlantHealth::isValidClass(int healthClass) const {
    return (healthClass >= 1 && healthClass <= 6);
}
