#ifndef ATMCORRECTIONMAINWINDOW_H
#define ATMCORRECTIONMAINWINDOW_H

#include <QMainWindow>

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

private slots:
    void on_pushButton_calculateBlack_clicked();
    void showResult(result_values);

private:
    Ui::AtmCorrectionMainWindow* ui;
    // void solve_dark_pixels(const QString& satellite_name,
    // const QVector<double>& dark_pixels);
signals:
    void resolveBlack(const QString& satellite_name,
                      const QVector<double>& dark_pixels);
};

#endif  // ATMCORRECTIONMAINWINDOW_H
