#include "GuiModules/UasvViewWindow.h"

#include <QApplication>
#include <QDateTime>
#include <QTextStream>
#include <BaseTools/QrcFilesRestorer.h>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QrcFilesRestorer::restoreFilesFromQrc(":/4Release/_restoring/");
    UasvViewWindow w;
    w.show();
    return a.exec();
}
