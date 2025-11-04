#ifndef ICON_GENERATOR_H
#define ICON_GENERATOR_H

#include <QIcon>
#include <QSize>
#include <QColor>
#include <QVector>

namespace iut{

QIcon createIcon(int r, int g, int b, QSize size = QSize(100, 100));



QVector<QColor> generateOrangeShades(int N);


}

#endif // ICON_GENERATOR_H
