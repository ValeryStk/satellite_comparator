#ifndef QCUSTOMPLOT_PRO_H
#define QCUSTOMPLOT_PRO_H

#include <QObject>

#include "qcustomplot.h"

class QCustomPlot_pro : public QCustomPlot {
    Q_OBJECT
public:
    explicit QCustomPlot_pro(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
};

#endif  // QCUSTOMPLOT_PRO_H
