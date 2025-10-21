#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>

class ViewSyncManager;

class ImageViewer : public QGraphicsView {
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    void setImage(const QPixmap& pixmap);
    void centerOnPixel(int x, int y);
    void connectSync(ViewSyncManager* sync);

signals:
    void localZoomChanged(double scale);
    void localCenterChanged(QPointF center);

public slots:
    void applyZoom(double scale);
    void centerOnPoint(const QPointF& point);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem* imageItem;
    QGraphicsLineItem* crosshairH;
    QGraphicsLineItem* crosshairV;
    QGraphicsRectItem* centerRect;

    void updateCrosshair(int x, int y);
};

#endif // IMAGEVIEWER_H
