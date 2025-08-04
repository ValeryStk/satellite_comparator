#include "satellite_graphics_view.h"
#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>


SatelliteGraphicsView::SatelliteGraphicsView(QWidget *parent)
{
    setParent(parent);
    isSignal = false;
    polygonItem = new QGraphicsPolygonItem();
    polygonItem->setPen(QPen(Qt::DashLine));

}

void SatelliteGraphicsView::setIsSignal(bool value)
{
    isSignal = value;
}

void SatelliteGraphicsView::setUp()
{
    if(scene())
    scene()->addItem(polygonItem);
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

/*void SatelliteGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        polygon << scenePos;
        polygonItem->setPolygon(polygon);
    } else if (event->button() == Qt::RightButton) {
        //selectItems();
        polygon.clear();
        polygonItem->setPolygon(polygon);
    } if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        polygon << scenePos;
        polygonItem->setPolygon(polygon);
    } else if (event->button() == Qt::RightButton) {
        //selectItems();
        polygon.clear();
        polygonItem->setPolygon(polygon);
    }
}*/

