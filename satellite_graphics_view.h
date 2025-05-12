#ifndef SATELLITE_GRAPHICS_VIEW_H
#define SATELLITE_GRAPHICS_VIEW_H

#include <QObject>
#include <QGraphicsView>

class SatelliteGraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    SatelliteGraphicsView(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void pointChanged(QPointF point);

};

#endif // SATELLITE_GRAPHICS_VIEW_H
