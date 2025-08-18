#ifndef WAVESRANGEDIALOG_H
#define WAVESRANGEDIALOG_H

#include <QDialog>
#include <bekas/GuiModules/SpectrumWidgets/SpectrPlotterWidget.h>

namespace Ui {
class WavesRangeDialog;
}

class WavesRangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WavesRangeDialog(QWidget *parent, QVector<double> waves, QVector<double> values,
                              double maxValue, db_json::BandUnits bandsUnits, db_json::SpectrumUnits spUnits,
                              double minWave = 0, double maxWave = 0);
    ~WavesRangeDialog();

    double maxWave() const;

    double minWave() const;
    void setMinWave(double newMinWave);

    void setMaxWave(double newMaxWave);

private slots:
    void on_horizontalSliderMinBand_sliderMoved(int position);

    void on_horizontalSliderMaxBand_sliderMoved(int position);

    void on_doubleSpinBoxStartWave_editingFinished();

    void on_doubleSpinBoxEndWave_editingFinished();

    void keyPressEvent(QKeyEvent* event) override;

private:
    /**
     * @brief setupGui  Function that setups GUI
     */
    void setupGui(QVector<double> &waves, QVector<double> &values, double maxValue,
                  db_json::BandUnits bandsUnits, db_json::SpectrumUnits spUnits);

    void setupCurves();
    void drawMinLine();
    void drawMaxLine();

    Ui::WavesRangeDialog *ui;
    SpectrPlotterWidget *m_plotterWidget;   //!< Plotter widget
    double m_minWave;
    double m_maxWave;
    QCPItemStraightLine *m_minVerticalLine;
    QCPItemStraightLine *m_maxVerticalLine;
};

#endif // WAVESRANGEDIALOG_H
