#ifndef SATELLITE_GRAPHICS_VIEW_H
#define SATELLITE_GRAPHICS_VIEW_H

#include <QObject>
#include <QGraphicsView>


constexpr int Z_INDEX_ROI_AREA_POLYGON  = 9998;
constexpr int Z_INDEX_CROSS_SQUARE_CURSOR  = 9999;
constexpr int Z_INDEX_CROSS_SQUARE_CURSOR_TEXT  = 10000;


class SatelliteGraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    SatelliteGraphicsView(QWidget *parent = nullptr);

    void setIsSignal(bool value);
    void setUp();

private:
    bool isSignal = false;
    QPointF m_current_point;
    QPolygonF polygon;
    QGraphicsPolygonItem *polygonItem;
    QVector<int> unused_trick = {Z_INDEX_ROI_AREA_POLYGON,Z_INDEX_CROSS_SQUARE_CURSOR,Z_INDEX_CROSS_SQUARE_CURSOR_TEXT};

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void pointChanged(QPointF point);
    void sampleChanged(QPointF point);

};

#endif // SATELLITE_GRAPHICS_VIEW_H
