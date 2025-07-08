#include "icon_generator.h"

#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QByteArray>
#include <QString>
#include <QImage>
#include <Qt>

namespace iut{
QIcon createIcon(int r, int g, int b, int a, QSize size)
{
    QString svg = QString("<svg width='%1' height='%2'><rect x='10' y='10' width='80' height='80' fill='rgb(%3,%4,%5)'/></svg>"
       ).arg(size.width()).arg(size.height()).arg(r).arg(g).arg(b);
    QByteArray svgData = svg.toUtf8();
    QSvgRenderer renderer(svgData);

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    return QIcon(pixmap);
}
}
