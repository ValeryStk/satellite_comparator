#include "SpectrPlotterWidget.h"

SpectrPlotterWidget::SpectrPlotterWidget(QCustomPlot *customPlot) : QWidget()
{
    m_customPlot = customPlot;
    m_isNeedToShowSpectrum = true;
    m_isNeedToShowValues = false;
    m_isNeedToShowPoints = false;
    m_currentMaxValue = 0;
    m_minBand = -100000;
    m_maxBand = 100000;
    m_minBandIndex = 0;
    setupPlot();
}

void SpectrPlotterWidget::setupPlot()
{
    m_graphColor = QColor(82, 67, 100);
    m_customPlot->setInteraction(QCP::iRangeDrag, true);
    m_customPlot->setInteraction(QCP::iRangeZoom, true);
    m_customPlot->setInteraction(QCP::iSelectPlottables, true);

    m_customPlot->axisRect()->setBackground(Qt::white);
    m_customPlot->xAxis->setTickLabelRotation(30);
    m_customPlot->xAxis->ticker()->setTickCount(9);
    QFont axeFont = font();
    axeFont.setPointSize(12);
    axeFont.setBold(true);
    m_customPlot->xAxis->setLabelFont(axeFont);
    m_customPlot->yAxis->setLabelFont(axeFont);
    axeFont.setPointSize(11);
    axeFont.setBold(false);
    m_customPlot->xAxis->setTickLabelFont(axeFont);
    m_customPlot->yAxis->setTickLabelFont(axeFont);
    m_customPlot->yAxis->setLabel("Отсчеты АЦП");
    m_customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    setupLegend();

    m_textValues = new QCPItemText(m_customPlot);
    m_vertLine = new QCPItemStraightLine(m_customPlot);
    m_horizLine = new QCPItemStraightLine(m_customPlot);
    m_textValues->setVisible(false); // т.к. есть баг, что при создании item'ов они сразу отрисовываются в (0,0)
    m_vertLine->setVisible(false);
    m_horizLine->setVisible(false);
    m_spectrumUnits = db_json::SU_ADC;
}

void SpectrPlotterWidget::setupLegend()
{
    m_customPlot->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    m_customPlot->legend->setFont(legendFont);
    m_customPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));
}

void SpectrPlotterWidget::fillShowingBands()
{
    m_showingBands.clear();
    int index = 0;
    while (m_bands.at(index) < m_minBand){
        index++;
    }
    m_minBandIndex = index;
    while(index < m_bands.count()){
        if(m_bands.at(index) < m_maxBand){
            m_showingBands.append(m_bands.at(index));
        }else{
            break;
        }
        index++;
    }
}

void SpectrPlotterWidget::updateSpectrumView()
{
    m_customPlot->xAxis->setRange(m_showingBands.first(), m_showingBands.last());
    m_customPlot->yAxis->setRange(0, m_currentMaxValue);
    m_customPlot->replot();
}

const QVector<double> &SpectrPlotterWidget::showingBands() const
{
    return m_showingBands;
}

void SpectrPlotterWidget::setLegendVisible(bool isVisible)
{
    m_customPlot->legend->setVisible(isVisible);
}

const QVector<double> &SpectrPlotterWidget::showingValues() const
{
    return m_showingValues;
}

int SpectrPlotterWidget::minBandIndex() const
{
    return m_minBandIndex;
}

double SpectrPlotterWidget::currentMaxValue() const
{
    return m_currentMaxValue;
}

void SpectrPlotterWidget::setBandsRange(double minBand, double maxBand)
{
    m_minBand = minBand;
    m_maxBand = maxBand;
    fillShowingBands();
    updateSpectrumView(m_values, m_currentMaxValue, false, m_customPlot->graph(0)->name());
}

db_json::BandUnits SpectrPlotterWidget::bandUnits() const
{
    return m_bandUnits;
}

db_json::SpectrumUnits SpectrPlotterWidget::getSpectrumUnits() const
{
    return m_spectrumUnits;
}

const QVector<double> &SpectrPlotterWidget::values() const
{
    return m_values;
}

const QVector<double> &SpectrPlotterWidget::bands() const
{
    return m_bands;
}

void SpectrPlotterWidget::setIsNeedToShowSpectrum(bool newIsNeedToShowSpectrum)
{
    m_isNeedToShowSpectrum = newIsNeedToShowSpectrum;
}

void SpectrPlotterWidget::setGraphValues(const db_json::SpectrumUnits &graphValues)
{
    m_spectrumUnits = graphValues;
}

void SpectrPlotterWidget::setIsNeedToShowPoints(bool isNeedToShowPoints)
{
    m_isNeedToShowPoints = isNeedToShowPoints;
    for(int i = 0; i < m_customPlot->graphCount();  i++){
        m_customPlot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, QPen(Qt::black, 0), QColor(90, 83, 138, 50), 10));
    }
    m_customPlot->replot();
}

void SpectrPlotterWidget::setZoomType(QFlags<Qt::Orientations::enum_type> orientation)
{
    m_customPlot->axisRect()->setRangeZoom(orientation);
}

void SpectrPlotterWidget::setBands(const QVector<double> bands, db_json::BandUnits bandsValues)
{
    m_bands = bands;
    m_bandUnits = bandsValues;
    fillShowingBands();
}

void SpectrPlotterWidget::changePlotStyle(QColor backgroundColor, QColor fontColor, QColor graphColor)
{
    m_customPlot->setBackground(backgroundColor);
    m_customPlot->axisRect()->setBackground(backgroundColor);
    m_customPlot->xAxis->setBasePen(QPen(fontColor, 1));
    m_customPlot->yAxis->setBasePen(QPen(fontColor, 1));
    m_customPlot->xAxis->setTickPen(QPen(fontColor, 1));
    m_customPlot->yAxis->setTickPen(QPen(fontColor, 1));
    m_customPlot->xAxis->setSubTickPen(QPen(fontColor, 1));
    m_customPlot->yAxis->setSubTickPen(QPen(fontColor, 1));
    m_customPlot->xAxis->setTickLabelColor(fontColor);
    m_customPlot->yAxis->setTickLabelColor(fontColor);
    m_customPlot->xAxis->grid()->setPen(QPen(fontColor, 1, Qt::DotLine));
    m_customPlot->yAxis->grid()->setPen(QPen(fontColor, 1, Qt::DotLine));
    m_customPlot->xAxis->setLabelColor(fontColor);
    m_customPlot->yAxis->setLabelColor(fontColor);
    m_graphColor = graphColor;
}

void SpectrPlotterWidget::lockSpectrumFeatures()
{
    disconnect(m_customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    disconnect(m_customPlot, SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(mouseMoveRequest(QMouseEvent*)));
}

void SpectrPlotterWidget::unlockSpectrumFeatures()
{
    connect(m_customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    connect(m_customPlot, SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(mouseMoveRequest(QMouseEvent*)));
}

QCustomPlot *SpectrPlotterWidget::customPlot() const
{
    return m_customPlot;
}

void SpectrPlotterWidget::showSpectrum(QVector<double> data, double max, bool isNeedToUpdate, QString spName)
{
    updateSpectrumView(data, max, isNeedToUpdate, spName);
}

void SpectrPlotterWidget::showSpectrum(QVector<unsigned int> data, unsigned int max, bool isNeedToUpdate, QString spName)
{
    updateSpectrumView(data, max, isNeedToUpdate, spName);
}

void SpectrPlotterWidget::showSpectrum(QVector<unsigned short> data, unsigned short max, bool isNeedToUpdate, QString spName)
{
    updateSpectrumView(data, max, isNeedToUpdate, spName);
}

void SpectrPlotterWidget::showSpectrum(QVector<unsigned long> data, unsigned long max, bool isNeedToUpdate, QString spName)
{
    updateSpectrumView(data, max, isNeedToUpdate, spName);
}

void SpectrPlotterWidget::on_actionShowValues_triggered(bool checked)
{
    m_isNeedToShowValues = checked;
    m_textValues->setVisible(checked);
    m_vertLine->setVisible(checked);
    m_horizLine->setVisible(checked);
}

void SpectrPlotterWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
        m_customPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    if(event->key() == Qt::Key_Control)
        m_customPlot->axisRect()->setRangeZoom(Qt::Vertical);
}

void SpectrPlotterWidget::keyReleaseEvent(QKeyEvent *event)
{
    m_customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    QWidget::keyReleaseEvent(event);
}

void SpectrPlotterWidget::contextMenuRequest(QPoint pos)
{
    Q_UNUSED(pos);
}

void SpectrPlotterWidget::mouseMoveRequest(QMouseEvent *e)
{
    double x = m_customPlot->xAxis->pixelToCoord(e->x());
    double y = m_customPlot->yAxis->pixelToCoord(e->y());
    if(m_isNeedToShowValues)
    {
        m_textValues->position->setCoords(x, m_customPlot->yAxis->pixelToCoord(e->y() - 10));
        m_textValues->setText(QString("x:%1   y:%2").arg(x).arg(y));
        m_textValues->setFont(QFont(font().family(), 10,QFont::Bold));
        m_horizLine->point1->setCoords(x,y);
        m_horizLine->point2->setCoords(0,y);
        m_vertLine->point1->setCoords(x,y);
        m_vertLine->point2->setCoords(x,0);
        m_vertLine->setSelectable(false);
        m_horizLine->setSelectable(false);
        m_customPlot->replot();
    }
}
