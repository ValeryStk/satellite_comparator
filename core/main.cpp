#include "main_window_satellite_comparator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    translator.load("app_ru.qm");
    qApp->installTranslator(&translator);
    MainWindowSatelliteComparator w;
    w.show();
    return a.exec();
}
