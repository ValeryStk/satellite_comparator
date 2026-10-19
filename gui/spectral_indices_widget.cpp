#include "spectral_indices_widget.h"

#include <QVBoxLayout>

#include "health_ranges.h"

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
    QVector<QColor> colors;
    for (int i = 0; i < indexNames.size(); ++i) {
        x[i] = i + 1;
        y[i] = m_indices[indexNames[i]];
        PlantHealth ph(indexNames[i].toStdString().c_str());
        int healthClass = ph.getHealthClass(y[i]);
        colors.append(
            QColor(QString::fromStdString(ph.getClassColor(healthClass))));
    }

    for (int i = 0; i < indexNames.size(); ++i) {
        // Столбец
        QCPBars *bars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
        QVector<double> singleX = {x[i]};
        QVector<double> singleY = {y[i]};
        bars->setData(singleX, singleY);
        bars->setPen(QPen(Qt::NoPen));
        bars->setBrush(QBrush(colors[i]));
        bars->setWidth(0.8);
        bars->setName(indexNames[i]);

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
    m_plot->yAxis->setRange(-0.5, 2.5);
    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    for (int i = 0; i < indexNames.size(); ++i) {
        ticker->addTick(i + 1, indexNames[i]);
    }
    m_plot->xAxis->setTicker(ticker);

    m_plot->legend->setVisible(true);
    m_plot->legend->clear();
    createLegend();
    m_plot->replot();
}

void SpectralIndicesWidget::setupUI() {
    auto *layout = new QVBoxLayout(this);
    m_plot = new QCustomPlot_pro(this);
    QSize size(300, 200);
    this->setMinimumSize(size);
    layout->addWidget(m_plot);
}

void SpectralIndicesWidget::setupPlot() {
    m_plot->xAxis->setLabel("Индекс");
    m_plot->yAxis->setLabel("Значение");
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->legend->setVisible(true);
    m_plot->axisRect()->setupFullAxesBox();

    // Элементы легенды — в одну строку, без переноса.
    // Порядок вызовов важен для уже существующих элементов.
    m_plot->legend->setWrap(4);
    m_plot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst, true);

    // Уменьшаем место, занимаемое каждым пунктом
    m_plot->legend->setIconSize(12, 8);
    m_plot->legend->setIconTextPadding(1);
    m_plot->legend->setColumnSpacing(5);
    m_plot->legend->setMargins(QMargins(2, 2, 2, 2));

    QFont font = m_plot->legend->font();
    font.setPointSize(8);
    m_plot->legend->setFont(font);

    // Переносим легенду из области графика в строку под ним.
    // Для стандартного layout: график расположен в ячейке (0, 0).
    m_plot->plotLayout()->addElement(1, 0, m_plot->legend);
    // Нижняя строка занимает минимально необходимую высоту.
    m_plot->plotLayout()->setRowStretchFactor(1, 0.001);
    // Необязательно: убираем рамку и увеличиваем расстояние между пунктами.
    m_plot->legend->setBorderPen(Qt::NoPen);
    m_plot->legend->setColumnSpacing(15);

    createLegend();
}

void SpectralIndicesWidget::createLegend() {
    // Добавляем заголовок-разделитель (опционально)
    auto *titleBars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
    titleBars->setName("Классы здоровья:");
    titleBars->setPen(QPen(Qt::NoPen));
    titleBars->setBrush(QBrush(Qt::transparent));
    titleBars->addToLegend(m_plot->legend);

    for (int i = 0; i < sam::HEALTH_CLASSES::NUMBER_OF_CLASSES; ++i) {
        auto *legendBars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
        legendBars->setName(QString(sam::kHealthClassesRussian[i]));
        legendBars->setBrush(QBrush(QColor(sam::kHealthClassesColors[i])));
        legendBars->setPen(QPen(Qt::NoPen));
        legendBars->addToLegend(m_plot->legend);
    }
}
