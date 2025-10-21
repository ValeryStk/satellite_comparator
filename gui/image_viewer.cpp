#include "image_viewer.h"

#include <QWheelEvent>
#include <QPen>
#include "view_sync_manager.h"

ImageViewer::ImageViewer(QWidget* parent)
    : QGraphicsView(parent), scene(new QGraphicsScene(this)),
      imageItem(nullptr),
      crosshairH(nullptr),
      crosshairV(nullptr),
      centerRect(nullptr) {
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void ImageViewer::setImage(const QPixmap& pixmap) {
    scene->clear();
    imageItem = scene->addPixmap(pixmap);
    scene->setSceneRect(pixmap.rect());
}

void ImageViewer::wheelEvent(QWheelEvent* event) {
    const double scaleFactor = 1.15;
    double factor = (event->angleDelta().y() > 0) ? scaleFactor : 1.0 / scaleFactor;
    scale(factor, factor);

    double newScale = transform().m11(); // предполагаем равномерный масштаб
    emit localZoomChanged(newScale);
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    emit localCenterChanged(scenePos);
    centerOnPoint(scenePos);
}

void ImageViewer::centerOnPixel(int x, int y) {
    centerOn(x, y);
    updateCrosshair(x, y);
    //emit localCenterChanged(QPointF(x, y));
}

void ImageViewer::updateCrosshair(int x, int y) {
    QPen pen(Qt::red);
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    pen.setCosmetic(true); // не масштабируется

    if (crosshairH) scene->removeItem(crosshairH);
    if (crosshairV) scene->removeItem(crosshairV);
    if (centerRect) scene->removeItem(centerRect);

    crosshairH = scene->addLine(scene->sceneRect().left(), y,
                                scene->sceneRect().right(), y, pen);
    crosshairV = scene->addLine(x, scene->sceneRect().top(),
                                x, scene->sceneRect().bottom(), pen);

    crosshairH->setOpacity(0.5);
    crosshairV->setOpacity(0.5);

    // Размер квадрата
    const int rectSize = 20;
    QRectF rect(x - rectSize / 2, y - rectSize / 2, rectSize, rectSize);

    // Создаем белый полупрозрачный квадрат

    centerRect = scene->addRect(rect, QPen(Qt::white,3), Qt::NoBrush);
    centerRect->setOpacity(0.3); // полупрозрачность
}

void ImageViewer::connectSync(ViewSyncManager* sync) {
    connect(sync, &ViewSyncManager::zoomChanged, this, &ImageViewer::applyZoom);
    connect(sync, &ViewSyncManager::centerChanged, this, &ImageViewer::centerOnPoint);

    connect(this, &ImageViewer::localZoomChanged, sync, &ViewSyncManager::setZoom);
    connect(this, &ImageViewer::localCenterChanged, sync, &ViewSyncManager::setCenter);
}

void ImageViewer::applyZoom(double scale) {
    QTransform t;
    t.scale(scale, scale);
    setTransform(t);
}

void ImageViewer::centerOnPoint(const QPointF& point) {
    centerOn(point);
    updateCrosshair(point.x(), point.y());
}

