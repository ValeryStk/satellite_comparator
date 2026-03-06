#ifndef PLANTHEALTH_H
#define PLANTHEALTH_H

#include <map>
#include <string>
#include <vector>

namespace sam {

extern const char kSpectralIndexNDVI[];
extern const char kSpectralIndexSWVI[];
extern const char kSpectralIndexDSWI[];
extern const char kSpectralIndexEVI[];
extern const char kSpectralIndexNBR[];
extern const char kSpectralIndexNDSWIR[];
extern const char kSpectralIndexNBRSWIR[];

extern const char* kHealthClassesEnglish[];
extern const char* kHealthClassesRussian[];
extern const char* kHealthClassesColors[];

enum HEALTH_CLASSES {
    HEALTHY,
    WEAKENED,
    SEVERELY_WEAKENED,
    DYING,
    FRESH_DEADWOOD,
    OLD_DEADWOOD,
    // NUMBER_OF_CLASSES
    NUMBER_OF_CLASSES
};

}  // namespace sam

/**
 * Класс для оценки состояния растений по вегетационным индексам
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
    std::string currentIndex;  // Текущий выбранный индекс
    std::map<std::string, std::map<int, double>>
        indexRanges;  // Пороги для каждого индекса
    std::map<int, std::string> classNames;   // Названия классов
    std::map<int, std::string> classColors;  // Цвета классов (HEX)

    // Инициализация диапазонов для разных индексов
    void initDefaultRanges();

public:
    // Единый конструктор с параметром
    explicit PlantHealth(const std::string& indexName);
    ~PlantHealth();

    // Методы для работы с индексами
    bool setIndex(const std::string& indexName);
    std::string getCurrentIndex() const;
    std::vector<std::string> getAvailableIndices() const;

    // Основные методы для работы с классами состояния
    int getHealthClass(double indexValue) const;
    double getThreshold(int healthClass) const;
    std::string getClassName(int healthClass) const;
    std::string getClassColor(int healthClass) const;

    // Методы для работы с порогами
    void setThreshold(int healthClass, double value);
    void resetToDefaults();
    void resetIndexToDefaults(const std::string& indexName);

    // Вспомогательные методы
    std::map<int, double> getAllThresholds() const;
    std::string getDescription(int healthClass) const;
    bool isValidClass(int healthClass) const;
};

#endif  // PLANTHEALTH_H
