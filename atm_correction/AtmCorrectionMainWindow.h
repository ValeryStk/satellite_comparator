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
    void setSunZenitAngle(const double value);
    void setSunAzimutAngle(const double value);
    void setCaptureZenitAngle(const double value);
    void setCaptureAzimutAngle(const double value);
    void updateBasePixel(QVector<double> pixel_bands);
    void updateSatelliteType(const QString& satName);
    void showAlbedoUnderCursor(QVector<double> speya_values);
    QVector<double> getAlbedoBySpeya(const QVector<double>& speya_values);

private slots:
    void on_pushButton_calculateBlack_clicked();
    void showResult(result_values);
    void on_comboBox_satellite_type_currentIndexChanged(const QString& arg1);

    void on_pushButton_CopyKsy_clicked();

    void on_pushButton_create_Image_clicked();

private:
    Ui::AtmCorrectionMainWindow* ui;
    BandsWidget* bands_widget;
    QCustomPlot* atm_params_plot;
    QVector<double> m_central_waves;
    QVector<double> base_pixel_speya_values;
    QString base_pixel_speya_valuesStr;
    result_values m_atm_cor_result;
    QMap<QString, QPoint> cellMap;
    QCustomPlot* fitting_plot;
    // Метод для получения вещественного числа из ячейки по имени
    double getCellValue(const QString& name);

    // Метод для записи вещественного числа в ячейку по имени
    void setCellValue(const QString& name, double value, int precision = 2);

    void setCellStringValue(const QString& name, const QString text);
    void updateInitialValues();

    // void solve_dark_pixels(const QString& satellite_name,
    // const QVector<double>& dark_pixels);
signals:
    void resolveBlack(const QString& satellite_name,
                      const QVector<double>& dark_pixels);
    void responseForCreatingImage();
};

#endif  // ATMCORRECTIONMAINWINDOW_H
