// spectral_indices_widget.cpp
#include "spectral_indices_widget.h"

#include <QVBoxLayout>

SpectralIndicesWidget::SpectralIndicesWidget(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    setupPlot();
}

void SpectralIndicesWidget::setIndices(const QMap<QString, double> &indices) {
    m_indices = indices;
    updateDisplay();
}

void SpectralIndicesWidget::updateDisplay() {
    if (m_indices.isEmpty()) {
        return;
    }

    m_plot->clearPlottables();
    m_plot->clearItems();

    QStringList indexNames = m_indices.keys();
    QVector<double> x(indexNames.size()), y(indexNames.size());

    for (int i = 0; i < indexNames.size(); ++i) {
        x[i] = i + 1;
        y[i] = m_indices[indexNames[i]];
    }

    QVector<QColor> colors = {
        QColor(255, 0, 0, 200),   QColor(0, 255, 0, 200),
        QColor(0, 0, 255, 200),   QColor(255, 255, 0, 200),
        QColor(255, 0, 255, 200), QColor(0, 255, 255, 200),
        QColor(255, 165, 0, 200), QColor(128, 0, 128, 200)};

    for (int i = 0; i < indexNames.size(); ++i) {
        // Столбец
        QCPBars *bars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
        QVector<double> singleX = {x[i]};
        QVector<double> singleY = {y[i]};
        bars->setData(singleX, singleY);
        bars->setPen(QPen(Qt::NoPen));
        bars->setBrush(QBrush(colors[i % colors.size()]));
        bars->setWidth(0.8);
        bars->setName(indexNames[i]);

        // Надпись над столбиком - ПРАВИЛЬНЫЙ СИНТАКСИС
        QCPItemText *valueLabel = new QCPItemText(m_plot);
        valueLabel->position->setType(QCPItemPosition::ptPlotCoords);
        valueLabel->position->setCoords(x[i], y[i] + 0.08);
        valueLabel->setText(QString::number(y[i], 'f', 3));
        valueLabel->setFont(QFont("Arial", 11, QFont::Bold));
        valueLabel->setPositionAlignment(Qt::AlignCenter);
    }

    m_plot->xAxis->setLabel("Индексы");
    m_plot->yAxis->setLabel("Значение");
    m_plot->xAxis->setRange(0.2, indexNames.size() + 0.8);
    m_plot->yAxis->setRange(-1.1, 1.3);

    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    for (int i = 0; i < indexNames.size(); ++i) {
        ticker->addTick(i + 1, indexNames[i]);
    }
    m_plot->xAxis->setTicker(ticker);

    m_plot->legend->setVisible(true);
    m_plot->replot();
}

void SpectralIndicesWidget::setupUI() {
    auto *layout = new QVBoxLayout(this);
    m_plot = new QCustomPlot(this);
    QSize size(400, 200);
    this->setMinimumSize(size);
    this->setMaximumSize(size);
    layout->addWidget(m_plot);
}

void SpectralIndicesWidget::setupPlot() {
    m_plot->xAxis->setLabel("Индекс");
    m_plot->yAxis->setLabel("Значение");
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->legend->setVisible(true);
    m_plot->axisRect()->setupFullAxesBox();
}
