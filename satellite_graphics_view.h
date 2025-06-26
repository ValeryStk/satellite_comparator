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

private:
    bool isSignal = false;

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void pointChanged(QPointF point);
    void sampleChanged(QPointF point);



    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // SATELLITE_GRAPHICS_VIEW_H
