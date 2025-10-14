#include "view_sync_manager.h"

ViewSyncManager::ViewSyncManager(QObject* parent)
    : QObject(parent) {}

void ViewSyncManager::setZoom(double scale) {
    if (scale != m_currentZoom) {
        m_currentZoom = scale;
        emit zoomChanged(scale);
    }
}

void ViewSyncManager::setCenter(const QPointF& center) {
    if (center != m_currentCenter) {
        m_currentCenter = center;
        emit centerChanged(center);
    }
}

double ViewSyncManager::currentZoom() const {
    return m_currentZoom;
}

QPointF ViewSyncManager::currentCenter() const {
    return m_currentCenter;
}
