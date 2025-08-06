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
    polygonItem->setPen(QPen(QBrush(QColor(Qt::red)),2,Qt::DashLine));

}

void SatelliteGraphicsView::setIsSignal(bool value)
{
    isSignal = value;
}

void SatelliteGraphicsView::setUp(QGraphicsScene *scene)
{
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setScene(scene);
    if(scene){
        polygonItem->setZValue(Z_INDEX_ROI_AREA_POLYGON);
        scene->addItem(polygonItem);
    }
}

void SatelliteGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = mapToScene(event->pos());
    m_current_point = pos;
    QGraphicsView::mouseMoveEvent(event);
    if(isSignal) emit pointChanged(pos);
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
    if(isSignal) emit sampleChanged(pos);
}

void SatelliteGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        polygon << m_current_point;
        polygonItem->setPolygon(polygon);
    }else if(event->key() == Qt::Key_Escape){
        polygon.clear();
        polygonItem->setPolygon(polygon);
        polygonItem->setBrush(QBrush());
    }else if(event->key() == Qt::Key_Return){
        polygonItem->setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
    }

    QGraphicsView::keyPressEvent(event);
}

void SatelliteGraphicsView::zoomIn()
{
    scale(1.2, 1.2);
}

void SatelliteGraphicsView::zoomOut()
{
    scale(0.8, 0.8);
}
