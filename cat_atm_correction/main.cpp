#include <CompMonitor.h>
#include <QrcFilesRestorer.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QSettings>
#include <QTextCodec>

void myMessageOutput(QtMsgType type, const QMessageLogContext& context,
                     const QString& msg) {
    QFile file(QCoreApplication::applicationDirPath() + "//atmoc.log");
    if (file.exists())
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    else
        file.open(QIODevice::WriteOnly | QIODevice::Text);
    QString time = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP 1251"));
    QString OutMessage = "";
    QString conFile = context.file;
    QString conLine = QString::number(context.line);
    QString contextText = context.function;
    QTextStream out(&file);

    switch (type) {
        case QtInfoMsg:
            OutMessage = msg;
            break;
        case QtDebugMsg:
            OutMessage = QString("Debug[%1]: %2 (%3:%4, %5)\n")
                             .arg(time, msg, conFile, conLine, contextText);
            break;
        case QtWarningMsg:
            OutMessage = QString("Warning[%1]: %2 (%3:%4, %5)\n")
                             .arg(time, msg, conFile, conLine, contextText);
            break;
        case QtCriticalMsg:
            OutMessage = QString("Critical[%1]: %2 (%3:%4, %5)\n")
                             .arg(time, msg, conFile, conLine, contextText);
            break;
        case QtFatalMsg:
            OutMessage = QString("Fatal[%1]: %2 (%3:%4, %5)\n")
                             .arg(time, msg, conFile, conLine, contextText);
            abort();
    }
    out << OutMessage;
    file.close();
}

int main(int argc, char* argv[]) {
    QSettings* m_settings =
        new QSettings(QDir::currentPath() + "/atmoc.ini", QSettings::IniFormat);
    if (m_settings->value("Debug/isDebug").toBool()) {
        qInstallMessageHandler(myMessageOutput);
    }
    QCoreApplication::setOrganizationName("IAPP of BSU");
    QrcFilesRestorer::restoreFilesFromQrc(":/_4release/_4restoring");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    qInfo() << "\n\n\n";
    qInfo() << "###############################################################"
               "###########\n";
    qInfo() << "#                                  CAT                         "
               "          #\n";
    qInfo() << "###############################################################"
               "###########\n\n";
    qInfo() << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    CompMonitor compMonitor;
    return app.exec();
}
