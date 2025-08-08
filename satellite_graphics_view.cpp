#include "satellite_graphics_view.h"

#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>



SatelliteGraphicsView::SatelliteGraphicsView(QWidget *parent)
{
    setParent(parent);
    isSignal = false;
    polygonItem = new QGraphicsPolygonItem();
    polygonItem->setPen(QPen(QBrush(QColor(Qt::red)),2,Qt::DashLine));

}

qreal SatelliteGraphicsView::getMaxZValue(QGraphicsScene* scene) {
    const QList<QGraphicsItem*> items = scene->items();  // предотвращает detach
    qreal maxZ = std::numeric_limits<qreal>::lowest();
    for (QGraphicsItem* item : items) {
        qreal z = item->zValue();
        if (z < Z_INDEX_ROI_AREA_POLYGON && z > maxZ) {
            maxZ = z;
        }
    }
    return (maxZ == std::numeric_limits<qreal>::lowest()) ? 0 : maxZ + 1;
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
        auto new_polygon = new QGraphicsPolygonItem(polygonItem->polygon());
        new_polygon->setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
        new_polygon->setZValue(getMaxZValue(scene()));
        scene()->addItem(new_polygon);
        auto stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd/hh:mm:ss");
        m_roi_polygons.insert(stamp,new_polygon);
        emit roiPolygonAdded(stamp);
        polygon.clear();
        polygonItem->setPolygon(polygon);
        polygonItem->setBrush(QBrush());
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

void SatelliteGraphicsView::show_roi_layer(const QString &id)
{
    m_roi_polygons[id]->setVisible(true);
}

void SatelliteGraphicsView::hide_roi_layer(const QString &id)
{
   m_roi_polygons[id]->setVisible(false);
}

void SatelliteGraphicsView::remove_roi_scene_layer(const QString &id)
{
    auto polygon_item = m_roi_polygons[id];
    scene()->removeItem(polygon_item);
    delete polygon_item;
    m_roi_polygons.remove(id);
}

void SatelliteGraphicsView::changeRoiColor(const QString &roi_id,
                                           const QColor &new_color)
{
    auto roi_item = m_roi_polygons.value(roi_id);
    if(!roi_item)return;
    roi_item->setBrush(QBrush(new_color));
    roi_item->update();
}

void SatelliteGraphicsView::setRoiSelectEffect(const QString &roi_id)
{
    auto roi_item = m_roi_polygons.value(roi_id);
    if(!roi_item)return;
    auto *effect = new QGraphicsDropShadowEffect;
    roi_item->setGraphicsEffect(effect);
    roi_item->update();
    this->centerOn(roi_item);
}
