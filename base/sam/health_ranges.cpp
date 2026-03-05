#include "health_ranges.h"

PlantHealth::PlantHealth() {
    // Диапазоны по умолчанию
    ranges[1] = 100.0;  // Healthy: 80-100
    ranges[2] = 79.9;   // Weakened: 60-79.9
    ranges[3] = 59.9;   // Severely Weakened: 40-59.9
    ranges[4] = 39.9;   // Dying: 20-39.9
    ranges[5] = 19.9;   // Fresh Deadwood: 5-19.9
    ranges[6] = 4.9;    // Old Deadwood: 0-4.9
}

void PlantHealth::setRange(int categoryId, double max) {
    ranges[categoryId] = max;
}

const Category& PlantHealth::getCategory(double value) const {
    for (auto it = ranges.begin(); it != ranges.end(); ++it) {
        if (value <= it.value()) {
            return categories[it.key() - 1];
        }
    }
    throw std::out_of_range("Значение вне диапазонов");
}

QColor PlantHealth::getColor(double value) const {
    return getCategory(value).color;
}

void PlantHealth::printRanges() const {
    qDebug() << "\nТекущие диапазоны:";
    for (auto it = ranges.begin(); it != ranges.end(); ++it) {
        const auto& cat = categories[it.key() - 1];
        double min = (it.key() == 1) ? 80.0 : ranges[it.key() + 1] + 0.1;
        qDebug() << cat.name << ":" << min << "-" << it.value()
                 << cat.color.name() << cat.color.name(QColor::HexRgb);
    }
    qDebug() << "";
}
