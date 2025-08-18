#include "cross_square.h"

CrossSquare::CrossSquare(int size) : m_size(size) {
    //setTransformOriginPoint(m_size / 2, m_size / 2);
}

QRectF CrossSquare::boundingRect() const
{
    return QRectF(-m_size / 2, -m_size / 2, m_size, m_size);
}


void CrossSquare::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *, QWidget *) {
    // Рисуем квадрат
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(255,0,0,128));
    painter->setPen(pen);
    painter->drawRect(-m_size / 2, -m_size / 2, m_size, m_size);
    pen.setWidth(1);
    painter->setPen(pen);
    // Вычисляем центры сторон
    QPointF center_top(0, -m_size / 2);
    QPointF center_bottom(0, m_size / 2);
    QPointF center_left(-m_size / 2, 0);
    QPointF center_right(m_size / 2, 0);

    // Рисуем линии перекрестия
    painter->drawLine(center_top, center_bottom);
    painter->drawLine(center_left, center_right);
}

