#ifndef ATMCORRECTIONMAINWINDOW_H
#define ATMCORRECTIONMAINWINDOW_H

#include <qcustomplot.h>

#include <QMainWindow>

#include "bands_widget.h"
#include "calculation_solver.h"

namespace Ui {
class AtmCorrectionMainWindow;
}

class AtmCorrectionMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AtmCorrectionMainWindow(QWidget* parent = nullptr);
    ~AtmCorrectionMainWindow();
    void setSunZenitAngle(const double);
    void updateBasePixel(QVector<double> pixel_bands);

private slots:
    void on_pushButton_calculateBlack_clicked();
    void showResult(result_values);
    void on_comboBox_satellite_type_currentIndexChanged(const QString& arg1);

private:
    Ui::AtmCorrectionMainWindow* ui;
    BandsWidget* bands_widget;
    QCustomPlot* atm_params_plot;
    QVector<double> base_pixel_speya_values;
    // void solve_dark_pixels(const QString& satellite_name,
    // const QVector<double>& dark_pixels);
signals:
    void resolveBlack(const QString& satellite_name,
                      const QVector<double>& dark_pixels);
};

#endif  // ATMCORRECTIONMAINWINDOW_H
