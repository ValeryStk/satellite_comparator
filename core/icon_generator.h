#ifndef ICON_GENERATOR_H
#define ICON_GENERATOR_H

#include <QIcon>
#include <QSize>

namespace iut{

   QIcon createIcon(int r, int g, int b, QSize size = QSize(100, 100));

}

#endif // ICON_GENERATOR_H
