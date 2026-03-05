#ifndef PLANTHEALTH_H
#define PLANTHEALTH_H

#include <map>
#include <string>

/**
 * Класс для оценки состояния растений по вегетационному индексу
 *
 * Классы состояния (от 1 до 6):
 * 1 - Healthy (здоровые)
 * 2 - Weakened (ослабленные)
 * 3 - Severely Weakened (сильно ослабленные)
 * 4 - Dying (усыхающие)
 * 5 - Fresh Deadwood (свежий сухостой)
 * 6 - Old Deadwood (старый сухостой)
 */
class PlantHealth {
private:
    std::map<int, double> ranges;  // Пороговые значения для классов состояния
    std::map<int, std::string> classNames;   // Названия классов
    std::map<int, std::string> classColors;  // Цвета классов (HEX)

public:
    // Конструктор и деструктор
    PlantHealth();
    ~PlantHealth();

    // Основные методы для работы с классами состояния
    int getHealthClass(double indexValue) const;
    double getThreshold(int healthClass) const;
    std::string getClassName(int healthClass) const;
    std::string getClassColor(int healthClass) const;

    // Методы для работы с порогами
    void setThreshold(int healthClass, double value);
    void resetToDefaults();

    // Вспомогательные методы
    std::map<int, double> getAllThresholds() const;
    std::string getDescription(int healthClass) const;
    bool isValidClass(int healthClass) const;
};

#endif  // PLANTHEALTH_H
