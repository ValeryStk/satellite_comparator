#ifndef PLANT_HEALTH_H
#define PLANT_HEALTH_H

#include <QColor>
#include <QMap>
#include <QString>
#include <QDebug>

// Структура категории (статичная)
struct Category {
    int id;
    QString name;
    QColor color;
};

// Класс для работы с диапазонами
class PlantHealth {
private:
    // Статичные категории с цветами
    const Category categories[6] = {
        {1, "Healthy", QColor(0, 128, 0)},      // #008000
        {2, "Weakened", QColor(144, 238, 144)},  // #90EE90
        {3, "Severely Weakened", QColor(154, 205, 50)}, // #9ACD32
        {4, "Dying", QColor(255, 215, 0)},       // #FFD700
        {5, "Fresh Deadwood", QColor(205, 133, 63)}, // #CD853F
        {6, "Old Deadwood", QColor(139, 69, 19)}  // #8B4513
    };
    
    // Диапазоны: ключ - ID категории, значение - макс. значение диапазона
    QMap<int, double> ranges;
    
public:
    // Конструктор с диапазонами по умолчанию
    PlantHealth();
    
    // Установить диапазон для категории
    void setRange(int categoryId, double max);
    
    // Получить категорию по значению
    const Category& getCategory(double value) const;
    
    // Получить цвет по значению
    QColor getColor(double value) const;
    
    // Показать текущие диапазоны
    void printRanges() const;
};

#endif