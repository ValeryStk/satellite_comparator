import json
import os
import matplotlib.pyplot as plt
import numpy as np


def load_high_res_spectrum(file_path):
    """Шаг 0: Загрузка высокоразрешающего спектра из текстового файла.
    
    Поддерживает формат: две колонки (длина волны и значение), разделенные пробелами/табуляцией.
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Файл со спектром не найден по пути: {file_path}")
        
    try:
        data = np.loadtxt(file_path, comments=['#', ';'])
        
        # Проверяем, что это матрица (ndim == 2) и в ней есть минимум 2 элемента в форме
        if data.ndim != 2 or data.shape[1] < 2:
            raise ValueError("Файл должен содержать как минимум две колонки (длина волны и значение).")
            
        waves = data[:, 0]
        reflectance = data[:, 1]
        
        print(f"Успешно загружен спектр из {file_path} ({len(waves)} точек, диапазон: {waves.min():.1f} - {waves.max():.1f} нм)")
        return waves, reflectance
        
    except Exception as e:
        print(f"Ошибка при чтении файла {file_path}: {e}")
        raise


def save_convolved_results(output_path, convolved_results):
    """Шаг 5: Экспорт результатов свертки в простой текстовый файл.
    
    Формат записи: центральная длина волны [табуляция] значение сигнала.
    """
    try:
        export_data = []
        for band_name, data in convolved_results.items():
            export_data.append([data["center_wave"], data["value"]])
        
        # Сортировка по возрастанию длины волны
        export_data = sorted(export_data, key=lambda x: x[0])
        export_data = np.array(export_data)
        
        np.savetxt(
            output_path, 
            export_data, 
            fmt=['%.4f', '%.5f'], 
            delimiter='\t',
            header="wavelength_nm\tconvolved_value",
            comments='# '
        )
        print(f" -> Результаты сохранены в: {output_path}")
        
    except Exception as e:
        print(f" -> Ошибка при сохранении результатов в файл {output_path}: {e}")


def process_single_satellite(
    satellite_name,
    bands_data,
    cropped_waves_1nm,
    cropped_reflectance_1nm,
    output_dir,
    crop_min,
    crop_max
):
    """Внутренний пайплайн обработки и визуализации для выбранного спутника."""
    convolved_results = {}
    bands_srf_to_plot = []

    print(f"\n[Обработка спутника: {satellite_name}]")
    
    for band in bands_data:
        band_name = band["physicalBand"]
        srf_responses = np.array(band["spectral_response"])
        w_min = band["wavelength_MIN_nm"]
        w_max = band["wavelength_MAX_nm"]

        band_waves = np.linspace(w_min, w_max, len(srf_responses))
        bands_srf_to_plot.append({"name": band_name, "waves": band_waves, "srf": srf_responses})

        if w_max < crop_min or w_min > crop_max:
            continue

        interpolated_high_res = np.interp(
            band_waves, cropped_waves_1nm, cropped_reflectance_1nm, left=0, right=0
        )

        valid_mask = (band_waves >= crop_min) & (band_waves <= crop_max)
        total_response = np.sum(srf_responses[valid_mask])

        if total_response == 0:
            continue

        coverage_pct = (total_response / np.sum(srf_responses)) * 100
        weighted_signal = np.sum(interpolated_high_res * srf_responses)
        convolved_value = weighted_signal / total_response

        convolved_results[band_name] = {
            "center_wave": band["wavelength_CENTRAL_nm"],
            "value": convolved_value,
            "coverage": coverage_pct,
        }

    if not convolved_results:
        print(f" -> Нет активных каналов спутника {satellite_name} в заданном диапазоне обрезки.")
        return

    # Сохраняем текстовый результат
    output_file_name = f"output_{satellite_name}.txt"
    output_path = os.path.join(output_dir, output_file_name)
    save_convolved_results(output_path, convolved_results)

    # Визуализация данных
    fig, ax1 = plt.subplots(figsize=(12, 6))

    (line1,) = ax1.plot(cropped_waves_1nm, cropped_reflectance_1nm, color="black", linewidth=2, label="Входной спектр (1 нм)")
    ax1.set_xlabel("Длина волны (нм)", fontsize=12)
    ax1.set_ylabel("Отражательная способность спектра", fontsize=12, color="black")
    ax1.tick_params(axis="y", labelcolor="black")
    ax1.grid(True, linestyle="--", alpha=0.5)

    ax2 = ax1.twinx()
    ax2.set_ylabel("Спектральная чувствительность канала (SRF)", fontsize=12, color="gray")
    ax2.tick_params(axis="y", labelcolor="gray")
    ax2.set_ylim(0, 1.1)

    for b_srf in bands_srf_to_plot:
        if b_srf["waves"].max() >= crop_min and b_srf["waves"].min() <= crop_max:
            ax2.fill_between(b_srf["waves"], b_srf["srf"], 0, alpha=0.10, label=f"Фильтр {b_srf['name']}")

    for band_name, data in convolved_results.items():
        ax1.scatter(data["center_wave"], data["value"], color="red", s=100, edgecolors="darkred", linewidths=1.5, zorder=5)
        ax1.text(data["center_wave"], data["value"] + 0.02, band_name, ha="center", va="bottom", fontsize=9, fontweight="bold", color="darkred")

    fake_scatter = ax1.scatter([], [], color="red", s=100, edgecolors="darkred", label="Результат свертки")
    ax1.legend([line1, fake_scatter], ["Входной спектр (1 нм)", "Результат свертки"], loc="upper right")

    plt.title(f"Спутник: {satellite_name} ({crop_min}-{crop_max} нм)", fontsize=14, fontweight="bold", pad=15)
    plt.xlim(crop_min - 20, crop_max + 20)
    plt.tight_layout()
    
    # Сохраняем график в файл
    plot_file_name = f"plot_{satellite_name}.png"
    plot_path = os.path.join(output_dir, plot_file_name)
    plt.savefig(plot_path, dpi=150)
    print(f" -> График сохранен в: {plot_path}")
    plt.close(fig)


def run_target_satellite_pipeline(
    spectrum_file_path,
    responses_folder_path,
    output_directory,
    satellite_type,
    crop_min=None,
    crop_max=None,
):
    """Пайплайн спектральной свертки под конкретный тип спутника."""
    # --- ШАГ 0: ЗАГРУЗКА ВНЕШНЕГО СПЕКТРА ---
    high_res_waves, high_res_reflectance = load_high_res_spectrum(spectrum_file_path)

    # --- ШАГ 1: ИНТЕРПОЛЯЦИЯ ИСХОДНОГО СПЕКТРА К 1 НМ ---
    orig_min = int(np.ceil(high_res_waves.min()))
    orig_max = int(np.floor(high_res_waves.max()))

    full_waves_1nm = np.arange(orig_min, orig_max + 1, 1)
    full_reflectance_1nm = np.interp(full_waves_1nm, high_res_waves, high_res_reflectance)

    # --- ШАГ 2: ОБРЕЗКА ПОД ЗАДАННЫЙ ДИАПАЗОН ---
    if crop_min is None:
        crop_min = orig_min
    if crop_max is None:
        crop_max = orig_max

    crop_mask = (full_waves_1nm >= crop_min) & (full_waves_1nm <= crop_max)
    cropped_waves_1nm = full_waves_1nm[crop_mask]
    cropped_reflectance_1nm = full_reflectance_1nm[crop_mask]

    if len(cropped_waves_1nm) == 0:
        print(f"Ошибка: В диапазоне {crop_min}-{crop_max} нм нет данных!")
        return

    # Подготовка директории результатов
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    # Нормализуем имя для поиска (убираем возможный суффикс, переданный пользователем)
    clean_sat_name = satellite_type.lower()
    if clean_sat_name.endswith("_responses"):
        clean_sat_name = clean_sat_name[:-10]

    # Сканируем папку responses на наличие подходящего файла (регистронезависимо)
    file_path = None
    if os.path.exists(responses_folder_path):
        for f_name in os.listdir(responses_folder_path):
            f_name_lower = f_name.lower()
            if f_name_lower == f"{clean_sat_name}_responses.json" or f_name_lower == f"{clean_sat_name}.json":
                file_path = os.path.join(responses_folder_path, f_name)
                break

    if file_path is None:
        print(f"Ошибка: Файл конфигурации для спутника '{satellite_type}' не найден в папке '{responses_folder_path}'.")
        print(f"Убедитесь, что там лежит файл {clean_sat_name}_responses.json или {clean_sat_name}.json")
        return

    # Извлекаем красивое оригинальное имя спутника из имени файла для отчетов
    actual_file_name = os.path.basename(file_path)
    base_name, _ = os.path.splitext(actual_file_name)
    if base_name.lower().endswith("_responses"):
        final_sat_name = base_name[:-10]
    else:
        final_sat_name = base_name

    # --- ШАГ 3: ЗАГРУЗКА И РАСЧЕТ ---
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            bands_data = json.load(f)
            
        if not isinstance(bands_data, list):
            print(f"Ошибка в файле {file_path}: ожидался список каналов внутри JSON.")
            return
            
        process_single_satellite(
            satellite_name=final_sat_name,
            bands_data=bands_data,
            cropped_waves_1nm=cropped_waves_1nm,
            cropped_reflectance_1nm=cropped_reflectance_1nm,
            output_dir=output_directory,
            crop_min=crop_min,
            crop_max=crop_max
        )
        print(f"\n[Обработка для {final_sat_name} успешно завершена!]")
        
    except Exception as e:
        print(f"Ошибка при обработке спутника {final_sat_name}: {e}")


# --- НАСТРОЙКА ПУТЕЙ И ЗАПУСК ---

SPECTRUM_INPUT = "fsr.txt"        # Ваш текстовый файл спектра (2 колонки)
RESPONSES_DIR = "responses"       # Папка, где лежат ваши файлы чувствительности (*_responses.json)
OUTPUT_DIR = "results"            # Папка результатов

# Задаем тип спутника. Скрипт сам найдет соответствующий файл в папке responses.
# Можно указывать как "sentinel2A", так и "sentinel2A_responses"
SATELLITE_TYPE = "sentinel2A" 

CROP_MIN_NM = 430
CROP_MAX_NM = 950

run_target_satellite_pipeline(
    spectrum_file_path=SPECTRUM_INPUT,
    responses_folder_path=RESPONSES_DIR,
    output_directory=OUTPUT_DIR,
    satellite_type=SATELLITE_TYPE,
    crop_min=CROP_MIN_NM,
    crop_max=CROP_MAX_NM,
)
