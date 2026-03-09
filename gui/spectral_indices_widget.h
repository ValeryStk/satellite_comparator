// spectral_indices_widget.h
#ifndef SPECTRAL_INDICES_WIDGET_H
#define SPECTRAL_INDICES_WIDGET_H

#include <qcustomplot.h>

#include <QMap>
#include <QString>
#include <QWidget>

class SpectralIndicesWidget : public QWidget {
    Q_OBJECT

public:
    explicit SpectralIndicesWidget(QWidget *parent = nullptr);
    void setIndices(const QMap<QString, double> &indices);

public slots:
    void updateDisplay();

private:
    QCustomPlot *m_plot;
    QMap<QString, double> m_indices;

    void setupUI();
    void setupPlot();
    void createLegend();
};

#endif  // SPECTRAL_INDICES_WIDGET_H
