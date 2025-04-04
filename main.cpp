#include "main_window_satellite_comparator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindowSatelliteComparator w;
    w.show();
    return a.exec();
}
