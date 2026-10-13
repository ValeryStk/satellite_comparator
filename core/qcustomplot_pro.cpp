#include "qcustomplot_pro.h"

QCustomPlot_pro::QCustomPlot_pro(QWidget *parent) : QCustomPlot(parent) {
    setInteraction(QCP::iRangeZoom, true);
    setInteraction(QCP::iRangeDrag, true);
}

void QCustomPlot_pro::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ShiftModifier)
        axisRect()->setRangeZoom(Qt::Horizontal);
    else if (event->modifiers() & Qt::ControlModifier)
        axisRect()->setRangeZoom(Qt::Vertical);
    else
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    QCustomPlot::wheelEvent(event);
}
