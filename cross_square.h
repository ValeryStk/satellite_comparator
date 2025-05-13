#ifndef CROSS_SQUARE_H
#define CROSS_SQUARE_H

#include <QGraphicsItem>
#include <QPainter>

class CrossSquare : public QGraphicsItem {
public:
    explicit CrossSquare(int size);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    int m_size;
};

#endif // CROSS_SQUARE_H

