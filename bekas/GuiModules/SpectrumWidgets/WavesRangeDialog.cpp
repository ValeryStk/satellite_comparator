#include "WavesRangeDialog.h"
#include "ui_WavesRangeDialog.h"

WavesRangeDialog::WavesRangeDialog(QWidget *parent, QVector<double> waves, QVector<double> values, double maxValue,
                                   db_json::BandUnits bandsUnits, db_json::SpectrumUnits spUnits, double minWave, double maxWave) :
    QDialog(parent),
    ui(new Ui::WavesRangeDialog)
{
    ui->setupUi(this);
    ui->widgetSpectra->setFocus();
    setupGui(waves, values, maxValue, bandsUnits, spUnits);
    if((maxWave - minWave) > 10){
        m_minWave = minWave;
        m_maxWave = maxWave;
        ui->doubleSpinBoxStartWave->setValue(m_minWave);
        ui->doubleSpinBoxEndWave->setValue(m_maxWave);

        ui->horizontalSliderMinBand->setValue(99 * (m_maxWave  - m_minWave) /
                                              (m_maxWave  - m_plotterWidget->bands().first()));
        ui->horizontalSliderMaxBand->setValue(99 * (m_maxWave - m_minWave) /
                                              (m_plotterWidget->bands().last() - m_minWave));
        drawMinLine();
        drawMaxLine();
    }
}

WavesRangeDialog::~WavesRangeDialog()
{
    delete ui;
}

void WavesRangeDialog::setupGui(QVector<double> &waves, QVector<double> &values, double maxValue,
                                db_json::BandUnits bandsUnits, db_json::SpectrumUnits spUnits)
{
    this->setWindowTitle("Выбор границ отображаемых длин волн");
    m_plotterWidget = new SpectrPlotterWidget(ui->widgetSpectra);
    m_plotterWidget->setBands(waves, bandsUnits);
    m_plotterWidget->setGraphValues(spUnits);
    m_plotterWidget->setLegendVisible(false);
    m_plotterWidget->showSpectrum(values, maxValue, false);
    m_minWave = waves.first();
    m_maxWave = waves.last();
    setupCurves();
}

void WavesRangeDialog::setupCurves()
{
    m_minVerticalLine = new QCPItemStraightLine(m_plotterWidget->customPlot());
    m_maxVerticalLine = new QCPItemStraightLine(m_plotterWidget->customPlot());
    m_minVerticalLine->setVisible(true);
    m_maxVerticalLine->setVisible(true);
}

void WavesRangeDialog::drawMinLine()
{
    m_minVerticalLine->point1->setCoords(m_minWave, m_plotterWidget->customPlot()->yAxis->range().minRange);
    m_minVerticalLine->point2->setCoords(m_minWave, m_plotterWidget->customPlot()->yAxis->range().maxRange);
    m_plotterWidget->customPlot()->replot();
}

void WavesRangeDialog::drawMaxLine()
{
    m_maxVerticalLine->point1->setCoords(m_maxWave, m_plotterWidget->customPlot()->yAxis->range().minRange);
    m_maxVerticalLine->point2->setCoords(m_maxWave, m_plotterWidget->customPlot()->yAxis->range().maxRange);
    m_plotterWidget->customPlot()->replot();
}

void WavesRangeDialog::setMaxWave(double newMaxWave)
{
    m_maxWave = newMaxWave;
}

double WavesRangeDialog::minWave() const
{
    return m_minWave;
}

void WavesRangeDialog::setMinWave(double newMinWave)
{
    m_minWave = newMinWave;
}

double WavesRangeDialog::maxWave() const
{
    return m_maxWave;
}

void WavesRangeDialog::on_horizontalSliderMinBand_sliderMoved(int position)
{
    double percent = static_cast<double>(position) / 99;
    m_minWave = m_maxWave  - (m_maxWave - m_plotterWidget->bands().first()) * percent;
    ui->doubleSpinBoxStartWave->setValue(m_minWave);
    drawMinLine();
}

void WavesRangeDialog::on_horizontalSliderMaxBand_sliderMoved(int position)
{
    double percent = static_cast<double>(position) / 99;
    m_maxWave = m_minWave  + (m_plotterWidget->bands().last() - m_minWave) * percent;
    ui->doubleSpinBoxEndWave->setValue(m_maxWave);
    drawMaxLine();
}

void WavesRangeDialog::on_doubleSpinBoxStartWave_editingFinished()
{
    m_minWave = ui->doubleSpinBoxStartWave->value();
    ui->horizontalSliderMinBand->setValue(99 * (m_maxWave  - m_minWave) /
                                          (m_maxWave  - m_plotterWidget->bands().first()));
    drawMinLine();
}


void WavesRangeDialog::on_doubleSpinBoxEndWave_editingFinished()
{
    m_maxWave = ui->doubleSpinBoxEndWave->value();
    ui->horizontalSliderMaxBand->setValue(99 * (m_maxWave - m_minWave) /
                                          (m_plotterWidget->bands().last() - m_minWave));
    drawMaxLine();
}

void WavesRangeDialog::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()){
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if(ui->doubleSpinBoxEndWave->hasFocus() || ui->doubleSpinBoxStartWave->hasFocus()){
            ui->widgetSpectra->setFocus();
        }else{
            this->accept();
        }
        break;
    default:
        break;

    }
}

