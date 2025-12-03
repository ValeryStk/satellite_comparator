#include "main_window_satellite_comparator.h"

#include <QApplication>


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{


    QFile file(QCoreApplication::applicationDirPath()+"//sda.log");
    if (file.exists()) file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    else file.open(QIODevice::WriteOnly | QIODevice::Text);
    QString time = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP 1251"));
    QString OutMessage="";
    QTextStream out(&file);

    switch (type) {
    case QtInfoMsg:
        OutMessage = msg;
        break;
    case QtDebugMsg:
        OutMessage = QString("Debug[%1]: %2 (%3:%4, %5)\n").arg(time).arg(msg).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtWarningMsg:
        OutMessage = QString("Warning[%1]: %2 (%3:%4, %5)\n").arg(time).arg(msg).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtCriticalMsg:
        OutMessage = QString("Critical[%1]: %2 (%3:%4, %5)\n").arg(time).arg(msg).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtFatalMsg:
        OutMessage = QString("Fatal[%1]: %2 (%3:%4, %5)\n").arg(time).arg(msg).arg(context.file).arg(context.line).arg(context.function);
        abort();
    }


    out << OutMessage;

    file.close();

}



int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    qInfo()<<"###################\n";
    qInfo()<<"# Forest Guard    #\n";
    qInfo()<<"###################\n";
    QTranslator translator;
    translator.load("app_ru.qm");
    qApp->installTranslator(&translator);
    MainWindowSatelliteComparator w;
    w.show();
    return a.exec();
}
