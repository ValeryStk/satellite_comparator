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
    //qDebug() << "Cursor Position: x=" << pos.x() << ", y=" << pos.y(); // Отображаем координаты
    QGraphicsView::mouseMoveEvent(event);
    if(isSignal)
        emit pointChanged(pos);
}

void SatelliteGraphicsView::wheelEvent(QWheelEvent *event)
{
    constexpr qreal zoomFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(zoomFactor, zoomFactor);  // Zoom in
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);  // Zoom out
    }
}

void SatelliteGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
   QPointF pos = mapToScene(event->pos());
   if(isSignal)
   sampleChanged(pos);
}

