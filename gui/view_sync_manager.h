#pragma once

#include <QObject>
#include <QPointF>

class ViewSyncManager : public QObject {
    Q_OBJECT

public:
    explicit ViewSyncManager(QObject* parent = nullptr);

    void setZoom(double scale);
    void setCenter(const QPointF& center);

    double currentZoom() const;
    QPointF currentCenter() const;

signals:
    void zoomChanged(double scale);
    void centerChanged(QPointF center);

private:
    double m_currentZoom = 1.0;
    QPointF m_currentCenter;
};
