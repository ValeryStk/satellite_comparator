#include "satellite_graphics_view.h"
#include <QMouseEvent>
#include <QDebug>


SatelliteGraphicsView::SatelliteGraphicsView(QWidget *parent)
{
    setParent(parent);
    isSignal = false;
}

void SatelliteGraphicsView::setIsSignal(bool value)
{
    isSignal = value;
}

void SatelliteGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = mapToScene(event->pos()); // Получаем координаты в системе QGraphicsScene
    qDebug() << "Cursor Position: x=" << pos.x() << ", y=" << pos.y(); // Отображаем координаты
    QGraphicsView::mouseMoveEvent(event);
    if(isSignal)
    emit pointChanged(pos);
}

void SatelliteGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
   QPointF pos = mapToScene(event->pos());
   if(isSignal)
   sampleChanged(pos);
}

