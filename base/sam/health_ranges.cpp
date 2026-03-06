#include "health_ranges.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sam {
// clang-format off
extern const char kSpectralIndexNDVI[]     = "NDVI";
extern const char kSpectralIndexSWVI[]     = "SWVI";
extern const char kSpectralIndexDSWI[]     = "DSWI";
extern const char kSpectralIndexEVI[]      = "EVI";
extern const char kSpectralIndexNBR[]      = "NBR";
extern const char kSpectralIndexNDSWIR[]   = "NDSWIR";
extern const char kSpectralIndexNBRSWIR[]  = "NBRSWIR";
// clang-format on
}  // namespace sam

PlantHealth::PlantHealth(const std::string& indexName)
    : currentIndex(indexName) {
    initDefaultRanges();

    // Проверяем, что запрошенный индекс существует
    if (indexRanges.find(indexName) == indexRanges.end()) {
        std::cerr << "Warning: Index '" << indexName
                  << "' not found. Using NDVI as default." << std::endl;
        currentIndex = sam::kSpectralIndexNDVI;  // По умолчанию NDVI
    }

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

void PlantHealth::initDefaultRanges() {
    // Диапазоны для NDVI (Normalized Difference Vegetation Index)
    // Типичный диапазон: -1 до 1
    std::map<int, double> ndviRanges;
    ndviRanges[1] = 0.7;  // Healthy: > 0.7
    ndviRanges[2] = 0.5;  // Weakened: 0.5 - 0.7
    ndviRanges[3] = 0.3;  // Severely Weakened: 0.3 - 0.5
    ndviRanges[4] = 0.2;  // Dying: 0.2 - 0.3
    ndviRanges[5] = 0.1;  // Fresh Deadwood: 0.1 - 0.2
    ndviRanges[6] = 0.0;  // Old Deadwood: 0.0 - 0.1
    indexRanges[sam::kSpectralIndexNDVI] = ndviRanges;

    // Диапазоны для DSWI (Disease Stress Water Index)
    // Типичный диапазон: 0 до 2+
    std::map<int, double> dswiRanges;
    dswiRanges[1] = 1.5;  // Healthy: > 1.5
    dswiRanges[2] = 1.2;  // Weakened: 1.2 - 1.5
    dswiRanges[3] = 1.0;  // Severely Weakened: 1.0 - 1.2
    dswiRanges[4] = 0.8;  // Dying: 0.8 - 1.0
    dswiRanges[5] = 0.6;  // Fresh Deadwood: 0.6 - 0.8
    dswiRanges[6] = 0.4;  // Old Deadwood: 0.0 - 0.4
    indexRanges[sam::kSpectralIndexDSWI] = dswiRanges;

    // Диапазоны для SWVI (Shortwave Vegetation Index)
    // Типичный диапазон: 0 до 1
    std::map<int, double> swviRanges;
    swviRanges[1] = 0.8;  // Healthy: > 0.8
    swviRanges[2] = 0.6;  // Weakened: 0.6 - 0.8
    swviRanges[3] = 0.4;  // Severely Weakened: 0.4 - 0.6
    swviRanges[4] = 0.3;  // Dying: 0.3 - 0.4
    swviRanges[5] = 0.2;  // Fresh Deadwood: 0.2 - 0.3
    swviRanges[6] = 0.1;  // Old Deadwood: 0.0 - 0.1
    indexRanges[sam::kSpectralIndexSWVI] = swviRanges;

    // Диапазоны для EVI (Enhanced Vegetation Index)
    // Типичный диапазон: -1 до 1
    std::map<int, double> eviRanges;
    eviRanges[1] = 0.6;   // Healthy: > 0.6
    eviRanges[2] = 0.4;   // Weakened: 0.4 - 0.6
    eviRanges[3] = 0.2;   // Severely Weakened: 0.2 - 0.4
    eviRanges[4] = 0.1;   // Dying: 0.1 - 0.2
    eviRanges[5] = 0.05;  // Fresh Deadwood: 0.05 - 0.1
    eviRanges[6] = 0.0;   // Old Deadwood: 0.0 - 0.05
    indexRanges[sam::kSpectralIndexEVI] = eviRanges;

    // Диапазоны для NBR (Normalized Burn Ratio)
    // Типичный диапазон: -1 до 1
    std::map<int, double> nbrRanges;
    nbrRanges[1] = 0.4;   // Healthy: > 0.4
    nbrRanges[2] = 0.2;   // Weakened: 0.2 - 0.4
    nbrRanges[3] = 0.1;   // Severely Weakened: 0.1 - 0.2
    nbrRanges[4] = 0.0;   // Dying: 0.0 - 0.1
    nbrRanges[5] = -0.1;  // Fresh Deadwood: -0.1 - 0.0
    nbrRanges[6] = -0.2;  // Old Deadwood: < -0.2
    indexRanges[sam::kSpectralIndexNBR] = nbrRanges;

    // Диапазоны для NDSWIR (Normalized Difference SWIR)
    // Типичный диапазон: -1 до 1
    std::map<int, double> ndswirRanges;
    ndswirRanges[1] = 0.5;   // Healthy: > 0.5
    ndswirRanges[2] = 0.3;   // Weakened: 0.3 - 0.5
    ndswirRanges[3] = 0.2;   // Severely Weakened: 0.2 - 0.3
    ndswirRanges[4] = 0.1;   // Dying: 0.1 - 0.2
    ndswirRanges[5] = 0.0;   // Fresh Deadwood: 0.0 - 0.1
    ndswirRanges[6] = -0.1;  // Old Deadwood: < -0.1
    indexRanges[sam::kSpectralIndexNDSWIR] = ndswirRanges;

    // Диапазоны для NBRSWIR (NBR Shortwave Infrared)
    // Типичный диапазон: -1 до 1
    std::map<int, double> nbrswirRanges;
    nbrswirRanges[1] = 0.3;   // Healthy: > 0.3
    nbrswirRanges[2] = 0.1;   // Weakened: 0.1 - 0.3
    nbrswirRanges[3] = 0.0;   // Severely Weakened: 0.0 - 0.1
    nbrswirRanges[4] = -0.1;  // Dying: -0.1 - 0.0
    nbrswirRanges[5] = -0.2;  // Fresh Deadwood: -0.2 - -0.1
    nbrswirRanges[6] = -0.3;  // Old Deadwood: < -0.3
    indexRanges[sam::kSpectralIndexNBRSWIR] = nbrswirRanges;
}

PlantHealth::~PlantHealth() {}

bool PlantHealth::setIndex(const std::string& indexName) {
    auto it = indexRanges.find(indexName);
    if (it != indexRanges.end()) {
        currentIndex = indexName;
        return true;
    }
    return false;
}

std::string PlantHealth::getCurrentIndex() const { return currentIndex; }

std::vector<std::string> PlantHealth::getAvailableIndices() const {
    std::vector<std::string> indices;
    for (const auto& pair : indexRanges) {
        indices.push_back(pair.first);
    }
    return indices;
}

int PlantHealth::getHealthClass(double indexValue) const {
    auto indexIt = indexRanges.find(currentIndex);
    if (indexIt == indexRanges.end()) {
        return 6;  // Индекс не найден
    }

    const auto& ranges = indexIt->second;

    // Проверка на некорректные значения
    if (indexValue < ranges.at(6)) return 6;  // Ниже минимального порога

    // Определяем класс состояния по значению индекса
    // Идем от здорового к сухостою
    if (indexValue > ranges.at(1)) return 1;  // Healthy
    if (indexValue > ranges.at(2)) return 2;  // Weakened
    if (indexValue > ranges.at(3)) return 3;  // Severely Weakened
    if (indexValue > ranges.at(4)) return 4;  // Dying
    if (indexValue > ranges.at(5)) return 5;  // Fresh Deadwood
    return 6;                                 // Old Deadwood
}

double PlantHealth::getThreshold(int healthClass) const {
    auto indexIt = indexRanges.find(currentIndex);
    if (indexIt == indexRanges.end()) {
        return -1.0;
    }

    const auto& ranges = indexIt->second;
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
    if (!isValidClass(healthClass) || value < 0.0) {
        return;
    }

    auto indexIt = indexRanges.find(currentIndex);
    if (indexIt != indexRanges.end()) {
        indexIt->second[healthClass] = value;
    }
}

void PlantHealth::resetToDefaults() { resetIndexToDefaults(currentIndex); }

void PlantHealth::resetIndexToDefaults(const std::string& indexName) {
    auto defaultIt = indexRanges.find(indexName);
    if (defaultIt == indexRanges.end()) {
        return;
    }

    // Переинициализируем диапазоны для указанного индекса
    if (indexName == sam::kSpectralIndexNDVI) {
        defaultIt->second[1] = 0.7;
        defaultIt->second[2] = 0.5;
        defaultIt->second[3] = 0.3;
        defaultIt->second[4] = 0.2;
        defaultIt->second[5] = 0.1;
        defaultIt->second[6] = 0.0;
    } else if (indexName == sam::kSpectralIndexDSWI) {
        defaultIt->second[1] = 1.5;
        defaultIt->second[2] = 1.2;
        defaultIt->second[3] = 1.0;
        defaultIt->second[4] = 0.8;
        defaultIt->second[5] = 0.6;
        defaultIt->second[6] = 0.4;
    } else if (indexName == sam::kSpectralIndexSWVI) {
        defaultIt->second[1] = 0.8;
        defaultIt->second[2] = 0.6;
        defaultIt->second[3] = 0.4;
        defaultIt->second[4] = 0.3;
        defaultIt->second[5] = 0.2;
        defaultIt->second[6] = 0.1;
    } else if (indexName == sam::kSpectralIndexEVI) {
        defaultIt->second[1] = 0.6;
        defaultIt->second[2] = 0.4;
        defaultIt->second[3] = 0.2;
        defaultIt->second[4] = 0.1;
        defaultIt->second[5] = 0.05;
        defaultIt->second[6] = 0.0;
    } else if (indexName == sam::kSpectralIndexNBR) {
        defaultIt->second[1] = 0.4;
        defaultIt->second[2] = 0.2;
        defaultIt->second[3] = 0.1;
        defaultIt->second[4] = 0.0;
        defaultIt->second[5] = -0.1;
        defaultIt->second[6] = -0.2;
    } else if (indexName == sam::kSpectralIndexNDSWIR) {
        defaultIt->second[1] = 0.5;
        defaultIt->second[2] = 0.3;
        defaultIt->second[3] = 0.2;
        defaultIt->second[4] = 0.1;
        defaultIt->second[5] = 0.0;
        defaultIt->second[6] = -0.1;
    } else if (indexName == sam::kSpectralIndexNBRSWIR) {
        defaultIt->second[1] = 0.3;
        defaultIt->second[2] = 0.1;
        defaultIt->second[3] = 0.0;
        defaultIt->second[4] = -0.1;
        defaultIt->second[5] = -0.2;
        defaultIt->second[6] = -0.3;
    }
}

std::map<int, double> PlantHealth::getAllThresholds() const {
    auto indexIt = indexRanges.find(currentIndex);
    if (indexIt != indexRanges.end()) {
        return indexIt->second;
    }
    return std::map<int, double>();
}

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
