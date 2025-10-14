#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>

class ImageViewer : public QGraphicsView {
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    void setImage(const QPixmap& pixmap);
    void centerOnPixel(int x, int y);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem* imageItem;
    QGraphicsLineItem* crosshairH;
    QGraphicsLineItem* crosshairV;

    void updateCrosshair(int x, int y);
};

#endif // IMAGEVIEWER_H
