#ifndef SLIDERS_OF_IMAGE_CORRECTOR_H
#define SLIDERS_OF_IMAGE_CORRECTOR_H

#include <QWidget>
#include <QSlider>


namespace Ui {
class SlidersOfImageCorrector;
}

/**
 * @brief Класс предназначен для расчёта коэффициентов которые используются при регулировкe яркости и цветового баланса изображения
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
    double getCoefSaturation() const;                       //! Функция получения коэффициента насыщенности
    double getCoefLight() const;                            //! Функция получения коэффициента яркости
    void setDefaultValues();                                //! Функция установки значений слайдеров и коэффициентов нейтральными значениями
    QSlider* getLightSlider();                              //! Функция получения текущего значения слайдера яркости
    QSlider* getSaturationSlider();                         //! Функция получения текущего значения слайдера насыщенности

signals:
    void slidersWereChanged();                              //! Сигнал об изменении положения слайдеров

private slots:
    void onSaturationChanged();                             //! Слот изменения коэффициента насыщенности
    void onLightChanged();                                  //! Слот изменения коэффициента яркости
    void on_slider_light_actionTriggered(int action);       //! Слот обработки действия слайдера яркости при нажатии
    void on_slider_saturation_actionTriggered(int action);  //! Слот обработки действия слайдера насыщенности при нажатии

private:
    Ui::SlidersOfImageCorrector *ui;                        //!< Пользовательский интерфейс
    double coefSaturation;                                  //!< Коэффициент насыщенности цвета изображения
    double coefLight;                                       //!< Коэффициент яркости цвета изображения

};

#endif // SLIDERS_OF_IMAGE_CORRECTOR_H
