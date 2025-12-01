#ifndef ICON_GENERATOR_H
#define ICON_GENERATOR_H

#include <QIcon>
#include <QSize>
#include <QColor>
#include <QVector>

namespace iut{
//!
//! \brief Функция для генерирования изображения виде закрашеного квадрата
//! \param r комонента красного цвета
//! \param g компонента зелёного цвета
//! \param b компонента голубого цвета
//! \param size размер квадрата
//! \return QIcon Объект иконки квадрата с заливкой r g b, определённого размера
//!
QIcon createIcon(int r, int g, int b, QSize size = QSize(100, 100));


//!
//! \brief Функция генерирования N количества оттенков оранжевого цвета
//! \param N количество градаций оранжевого
//! \return  QVector<QColor> - Вектор цветов с N количеством оттенков оранжевого цвета
//!
QVector<QColor> generateOrangeShades(int N);


}

#endif // ICON_GENERATOR_H
