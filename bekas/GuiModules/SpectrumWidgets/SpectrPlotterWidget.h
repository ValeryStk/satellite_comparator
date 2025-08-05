#ifndef SPECTRPLOTTERWIDGET_H
#define SPECTRPLOTTERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include "qcustomplot.h"
#include "bekas/BaseTools/DBJson.h"
#include "text_constants.h"

/**
 * @brief The SpectrPlotterWidget class
 * Class for drawing specrtrums in widget using QCustomPlot
 */
class SpectrPlotterWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief SpectrPlotterWidget   Constructor
     * @param customPlot    Custom Plot
     */
    explicit SpectrPlotterWidget(QCustomPlot *customPlot = nullptr);

    /**
     * @brief customPlot    Function to get custom plot object
     * @return  Custom plot object
     */
    QCustomPlot *customPlot() const;

    /**
     * @brief changePlotStyle   Function to change plot style
     * @param backgroundColor   Needed background Color
     * @param fontColor         Needed font color
     * @param graphColor        Needed graph color
     */
    void changePlotStyle(QColor backgroundColor, QColor fontColor, QColor graphColor);

    /**
     * @brief lockSpectrumFeatures  Function to lock spectrum features
     */
    void lockSpectrumFeatures();

    /**
     * @brief unlockSpectrumFeatures    Function to unlock spectrum features
     */
    void unlockSpectrumFeatures();

    /**
     * @brief setIsNeedToShowPoints Function to set is need to show points on graph
     * @param isNeedToShowPoints    Is need to show points on graph
     */
    void setIsNeedToShowPoints(bool isNeedToShowPoints);

    /**
     * @brief setZoomType   Function to set zoom type
     * @param orientation   Zoom orientation (Horizontal | Vertical)
     */
    void setZoomType(QFlags<Qt::Orientations::enum_type> orientation);

    /**
     * @brief setGraphValues    Function to set graph values
     * @param graphValues   Needed values (ADC_VALUES | RFL_VALUES)
     */
    void setGraphValues(const db_json::SpectrumUnits &graphValues);

    /**
     * @brief setIsNeedToShowSpectrum   Function needed to set is it needed to update showing spectrum
     * @param newIsNeedToShowSpectrum   Is it needed to update showing spectrum
     */
    void setIsNeedToShowSpectrum(bool newIsNeedToShowSpectrum);

    const QVector<double> &bands() const;

    const QVector<double> &values() const;

    db_json::BandUnits bandUnits() const;

    db_json::SpectrumUnits getSpectrumUnits() const;

    double currentMaxValue() const;

    void setBandsRange(double minBand, double maxBand);

    int minBandIndex() const;

    const QVector<double> &showingValues() const;

    const QVector<double> &showingBands() const;

    void setLegendVisible(bool isVisible);

public slots:
    /**
     * @brief showSpectrum  Function to show spectrum
     * @param data  Spectrum data
     * @param max   Data maximum
     * @param isNeedToUpdate    Is need to update plot
     */
    void showSpectrum(QVector<double> data, double max, bool isNeedToUpdate, QString spName = "");
    void showSpectrum(QVector<unsigned int> data, unsigned int max, bool isNeedToUpdate, QString spName = "");
    void showSpectrum(QVector<unsigned short> data, unsigned short max, bool isNeedToUpdate, QString spName = "");
    void showSpectrum(QVector<unsigned long> data, unsigned long max, bool isNeedToUpdate, QString spName = "");

    /**
     * @brief on_actionShowValues_triggered Slot for 'actionShowValues' 'triggered' signal
     * @param checked
     */
    void on_actionShowValues_triggered(bool checked);

    /**
     * @brief setBands      Slot to set bands vector in plotter
     * @param bands         Vector to set
     * @param bandsValues   Bands Units (BAND_NUMBERS/WAVELENGTH)
     */
    void setBands(const QVector<double> bands, db_json::BandUnits bandsValues);

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    /**
     * \brief Функция-слот вызывается при вызове контекстного меню (ПКМ)
     * @param pos - точка в окне, на которой вызвано контекстное меню
     */
    void contextMenuRequest(QPoint pos);

    /**
     * \brief Функция-слот вызывается при движении мыши
     * @param e - событие мыши
     */
    void mouseMoveRequest(QMouseEvent* e);


signals:
    /**
     * @brief sendRequestForSpectrum    Signal for new spectrum
     */
    void sendRequestForSpectrum();

    /**
     * @brief sendUpdatedPicture    Signal to send spectrum pixmap to show
     * @param map   Spectrum Pixmap
     */
    void sendUpdatedPicture(QPixmap map);

private:
    /**
     * @brief setupPlot Function needed to setup GUI
     */
    void setupPlot();

    void setupLegend();

    void fillShowingBands();

    void updateSpectrumView();

    /**
     * @brief updateSpectrumView    Function to show spectrum in the plotter widget
     * @param data  Data to show
     * @param max   Maximum value in the data vector
     * @param isNeedToUpdate    Is it needed to update the data
     */
    template < typename T>
    void updateSpectrumView(QVector<T> data, T max, bool isNeedToUpdate, QString spName = ""){
        m_currentMaxValue = max;
        if(isNeedToUpdate){
            emit sendRequestForSpectrum();
        }

        if(m_isNeedToShowSpectrum){
            if(m_bands.count() == data.count()){
                m_customPlot->clearGraphs();
                m_customPlot->addGraph();

                QPen graphPen(m_graphColor);
                graphPen.setWidth(2);
                m_customPlot->graph(0)->setPen(QPen(graphPen));
                //qDebug()<<"bands count:"<<m_showingBands.count()<<m_bands.count();
                m_customPlot->graph(0)->setData(m_bands, fillSpectrumVector(data));
                m_customPlot->graph(0)->setName(spName);

                if(m_bandUnits == db_json::BU_NUMBERS){
                    m_customPlot->xAxis->setLabel("Номер канала");
                }else{
                    m_customPlot->xAxis->setLabel("Длина волны, нм");
                }

                if(m_spectrumUnits == db_json::SU_ADC){
                    m_customPlot->yAxis->setLabel("Значения АЦП");
                }else if (m_spectrumUnits == db_json::SU_RFL){
                    m_customPlot->yAxis->setLabel("КСЯ");
                }else{
                    m_customPlot->yAxis->setLabel(satc::kSpeyaXUnit);
                }

                if(m_isNeedToShowPoints)
                    m_customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, QPen(Qt::black, 0), QColor(40, 70, 255, 50), 10));

                m_customPlot->rescaleAxes(true);
                m_customPlot->replot();
            }
        }
    }

    /**
     * @brief fillSpectrumVector    Function to form the double vector from template vector
     * @param dataVector    Template data vector
     * @return  Double data vector
     */
    template <typename T>
    QVector<double> fillSpectrumVector(QVector<T> dataVector){
        m_values.clear();
        m_showingValues.clear();
        int index = 0;
        m_currentMaxValue = 0;
        foreach(T value, dataVector){
            m_values.append(value);
            if(index >= m_minBandIndex && m_showingValues.count() < m_showingBands.count()){
                m_showingValues.append(value);
                if(m_currentMaxValue < value)
                    m_currentMaxValue = value;
            }
            index++;
        }
        return m_values;
    }


    QCustomPlot *m_customPlot;          //!< Qcustom plot
    QCPItemText *m_textValues;          //!< Graph text values
    QCPItemStraightLine *m_vertLine;    //!< Vertical line in values showing mode
    QCPItemStraightLine *m_horizLine;   //!< Horizontal line in values showing mode
    QColor m_graphColor;                //!< Current color

    QVector <double> m_bands;           //!< Bands vector
    QVector <double> m_showingBands;    //!< Bands vector to show
    double m_minBand;
    double m_maxBand;
    int m_minBandIndex;
    QVector<double> m_values;           //!< Values vector
    QVector<double> m_showingValues;    //!< Showing values vector
    double m_currentMaxValue;

    db_json::BandUnits m_bandUnits;     //!< Vands values (BAND_NUMBERS | WAVELENGTH)
    db_json::SpectrumUnits m_spectrumUnits;  //!< Graph values (SU_ADC | SU_RFL | SU_BRIGHT)

    bool m_isNeedToShowSpectrum;    //!< Is need to update plot
    bool m_isNeedToShowValues;      //!< Is need to show values under mouse
    bool m_isNeedToShowPoints;      //!< Is need to show points on plot
};

#endif // SPECTRPLOTTERWIDGET_H
