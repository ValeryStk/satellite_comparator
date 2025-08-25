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
    friend class SlidersOfImageCorrectorUnitTests;

public:
    //! Конструктор
    explicit SlidersOfImageCorrector(QWidget *parent = nullptr);

    //! Деструктор
    ~SlidersOfImageCorrector();

    //! Функция получения коэффициента насыщенности
    double getCoefSaturation() const;

    //! Функция получения коэффициента яркости
    double getCoefLight() const;

    //! Функция установки значений слайдеров и коэффициентов нейтральными значениями
    void setDefaultValues();

    //! Функция получения текущего значения слайдера яркости
    QSlider* getLightSlider();

    //! Функция получения текущего значения слайдера насыщенности
    QSlider* getSaturationSlider();

signals:
    //! Сигнал об изменении положения слайдеров
    void slidersWereChanged();

private slots:
    //! Слот изменения коэффициента насыщенности
    void onSaturationChanged();

     //! Слот изменения коэффициента яркости
    void onLightChanged();

    //! Слот обработки действия слайдера яркости при нажатии
    void on_slider_light_actionTriggered(int action);

    //! Слот обработки действия слайдера насыщенности при нажатии
    void on_slider_saturation_actionTriggered(int action);

    void on_slider_light_valueChanged(int value);

    void on_slider_saturation_valueChanged(int value);


private:
    Ui::SlidersOfImageCorrector *ui;                        //!< Пользовательский интерфейс
    double coefSaturation;                                  //!< Коэффициент насыщенности цвета изображения
    double coefLight;                                       //!< Коэффициент яркости цвета изображения

    void showToolTip(QSlider* slider, const int value);

};

#endif // SLIDERS_OF_IMAGE_CORRECTOR_H
