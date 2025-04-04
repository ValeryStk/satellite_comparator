#ifndef MAINWINDOWSATELLITECOMPARATOR_H
#define MAINWINDOWSATELLITECOMPARATOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowSatelliteComparator; }
QT_END_NAMESPACE

class MainWindowSatelliteComparator : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowSatelliteComparator(QWidget *parent = nullptr);
    ~MainWindowSatelliteComparator();

private:
    Ui::MainWindowSatelliteComparator *ui;
};
#endif // MAINWINDOWSATELLITECOMPARATOR_H
