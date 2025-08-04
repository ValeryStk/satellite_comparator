#ifndef SATELLITE_GRAPHICS_VIEW_H
#define SATELLITE_GRAPHICS_VIEW_H

#include <QObject>
#include <QGraphicsView>

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
