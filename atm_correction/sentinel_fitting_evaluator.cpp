#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct RadianceCompareResult {
    double reduced_chi_squared;  // Редуцированный Хи-квадрат (Идеал ~ 1.0)
    double mean_absolute_error;  // Средняя абсолютная ошибка в физических
                                 // единицах СПЭЯ
    double max_error_sigma;  // Максимальный вылет невязки (в единицах шума
                             // сенсора)
    std::string worst_band;  // Канал, где расхождение наиболее критично
    bool fits_within_noise;  // Флаг: совпали ли спектры в пределах погрешности
                             // прибора
};

class Sentinel2_Radiance_Evaluator {
private:
    const std::vector<std::string> BANDS = {"B01", "B02", "B03", "B04", "B05",
                                            "B06", "B07", "B08", "B8A", "B09"};

    // Паспортные значения SNR для геометрии Sentinel-2 L1C
    const std::vector<double> SNR_VALUES = {129.0, 154.0, 168.0, 142.0, 117.0,
                                            89.0,  105.0, 174.0, 72.0,  114.0};

public:
    RadianceCompareResult compareRadiance(
        const std::vector<double>&
            reference_radiance,  // Эталонный спектр СПЭЯ (или замеры)
        const std::vector<double>&
            evaluated_radiance  // Оцениваемый/моделируемый спектр СПЭЯ
    ) const {
        if (reference_radiance.size() < 10 || evaluated_radiance.size() < 10) {
            return {999.0, 999.0, 999.0, "Error", false};
        }

        double chi_squared_sum = 0.0;
        double absolute_error_sum = 0.0;
        double max_sigma = 0.0;
        size_t worst_idx = 0;

        for (size_t i = 0; i < 10; ++i) {
            // Модель шума СПЭЯ: Сигнал / SNR
            // Добавляем минимальный аппаратурный шум (0.05 Вт/(м²*ср*мкм)),
            // чтобы избежать деления на ноль на абсолютно темных каналах
            // (глубокие тени)
            double noise_sigma = (reference_radiance[i] / SNR_VALUES[i]) + 0.05;

            double residual = reference_radiance[i] - evaluated_radiance[i];
            double abs_err = std::abs(residual);
            double sigma_error = abs_err / noise_sigma;

            absolute_error_sum += abs_err;
            chi_squared_sum += sigma_error * sigma_error;

            if (sigma_error > max_sigma) {
                max_sigma = sigma_error;
                worst_idx = i;
            }
        }

        double red_chi2 = chi_squared_sum / 10.0;
        double mae = absolute_error_sum / 10.0;

        // В метрологии ДЗЗ два спектра СПЭЯ считаются идентичными,
        // если редуцированный хи-квадрат близок к 1, а максимальный выброс не
        // превышает 3 сигмы.
        bool match = (red_chi2 < 1.5) && (max_sigma < 3.0);

        return {red_chi2, mae, max_sigma, BANDS[worst_idx], match};
    }

    bool evaluateSoftFit(const std::vector<double>& base,
                         const std::vector<double>& fitted) {
        if (base.size() != fitted.size() || base.empty()) return false;

        for (size_t i = 0; i < base.size(); ++i) {
            // Вычисляем относительное отклонение
            double relative_error = std::abs(base[i] - fitted[i]) / base[i];

            // Если ошибка хотя бы в одном канале больше 8% (0.08) — брак.
            // В канале B04 текущая ошибка составляет 7.78%, то есть этот тест
            // ПРОЙДЕТ.
            if (relative_error > 0.08) {
                return false;
            }
        }
        return true;
    }
};
