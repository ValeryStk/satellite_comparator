#ifndef MAIN_WINDOW_SATELLITE_COMPARATOR_H
#define MAIN_WINDOW_SATELLITE_COMPARATOR_H

#include <QMainWindow>
#include <sattelite_comparator.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowSatelliteComparator; }
QT_END_NAMESPACE

class MainWindowSatelliteComparator : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowSatelliteComparator(QWidget *parent = nullptr);
    ~MainWindowSatelliteComparator();

private slots:
    void on_pushButton_open_sat_header_clicked();

private:
    Ui::MainWindowSatelliteComparator *ui;
    SatteliteComparator* m_sat_comparator;
    void openHeaderData();
};
#endif // MAIN_WINDOW_SATELLITE_COMPARATOR_H
