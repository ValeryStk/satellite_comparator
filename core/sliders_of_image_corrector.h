#ifndef SLIDERS_OF_IMAGE_CORRECTOR_H
#define SLIDERS_OF_IMAGE_CORRECTOR_H

#include <QWidget>
#include <QSlider>


namespace Ui {
class SlidersOfImageCorrector;
}

/**
 * @brief Класс предназначен для расчёта коэффициентов которые используются при регулировки яркости и цветового баланса изображения
 */
class SlidersOfImageCorrector : public QWidget
{
    Q_OBJECT
    friend class UnitTests;

public:
    //! Конструктор
    explicit SlidersOfImageCorrector(QWidget *parent = nullptr);

    //! Деструктор
    ~SlidersOfImageCorrector();
    double getCoefSaturation() const; //! Функция для передачи коэффициента насыщенности другим классам
    double getCoefLight() const; //! Функция для передачи коэффициента яркости другим классам
    void setDefaultValues(); //! Функция для калибровки средних значений слайдеров и коэффициентов
    QSlider* getLightSlider(); //! Функция для использования слайдера яркости другими классами
    QSlider* getSaturationSlider(); //! Функция для использования слайдера насыщенности другими классами

signals:
    void slidersWereChanged(); //! Сигнал об изменении положения слайдера

private slots:
    void onSaturationChanged(); //! Слот изменения коэффициента насыщенности
    void onLightChanged(); //! Слот изменения коэффициента яркости
    void on_slider_light_actionTriggered(int action); //! Слот обработки действия слайдера яркости при нажатии
    void on_slider_saturation_actionTriggered(int action); //! Слот обработки действия слайдера насыщенности при нажатии

private:
    Ui::SlidersOfImageCorrector *ui; //!< Пользовательский интерфейс
    double coefSaturation; //!< Коэффициент насыщенности цвета изображения
    double coefLight;   //!< Коэффициент яркости цвета изображения

};

#endif // SLIDERS_OF_IMAGE_CORRECTOR_H
