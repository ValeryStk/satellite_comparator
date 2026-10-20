#include <QApplication>

#include "main_window_satellite_comparator.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context,
                     const QString &msg) {
    QFile file(QCoreApplication::applicationDirPath() + "//sda.log");
    if (file.exists())
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    else
        file.open(QIODevice::WriteOnly | QIODevice::Text);
    QString time = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

    QString OutMessage = "";
    QTextStream out(&file);

    // Автоматический выбор флага кодировки UTF-8 в зависимости от версии Qt
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");  // Для Qt5
#else
    out.setEncoding(QStringConverter::Utf8);  // Для Qt6
#endif

    switch (type) {
        case QtInfoMsg:
            OutMessage = msg;
            break;
        case QtDebugMsg:
            OutMessage = QString("Debug[%1]: %2 (%3:%4, %5)\n")
                             .arg(time)
                             .arg(msg)
                             .arg(context.file)
                             .arg(context.line)
                             .arg(context.function);
            break;
        case QtWarningMsg:
            OutMessage = QString("Warning[%1]: %2 (%3:%4, %5)\n")
                             .arg(time)
                             .arg(msg)
                             .arg(context.file)
                             .arg(context.line)
                             .arg(context.function);
            break;
        case QtCriticalMsg:
            OutMessage = QString("Critical[%1]: %2 (%3:%4, %5)\n")
                             .arg(time)
                             .arg(msg)
                             .arg(context.file)
                             .arg(context.line)
                             .arg(context.function);
            break;
        case QtFatalMsg:
            OutMessage = QString("Fatal[%1]: %2 (%3:%4, %5)\n")
                             .arg(time)
                             .arg(msg)
                             .arg(context.file)
                             .arg(context.line)
                             .arg(context.function);
            abort();
    }

    out << OutMessage;

    file.close();
}

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // НАСТРОЙКА КИРИЛЛИЦЫ ДЛЯ КОНСОЛИ WINDOWS
#ifdef Q_OS_WIN
    // Устанавливаем UTF-8 для вывода в консоль Windows (если терминал
    // поддерживает UTF-8)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Настраиваем Qt на системную локаль
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));
    // qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    QString filePath;
    int contextId = -1;

    // Настраиваем стандартный парсер Qt
    QCommandLineParser parser;
    parser.setApplicationDescription("ForestGuard");
    parser.addHelpOption();  // Автоматически добавит поддержку -h и --help

    // Регистрируем опцию --file <value>
    QCommandLineOption fileOption("file", "Путь к файлу", "path");
    parser.addOption(fileOption);

    // Регистрируем опцию --context <value>
    QCommandLineOption contextOption("context", "ID контекста", "id");
    parser.addOption(contextOption);

    // Безопасно парсим Юникод-аргументы от ОС
    parser.process(a);

    // Извлекаем значения
    if (parser.isSet(fileOption)) {
        filePath = parser.value(
            fileOption);  // Кавычки убирать не нужно, Qt сделает всё сам
    }

    if (parser.isSet(contextOption)) {
        contextId = parser.value(contextOption).toInt();
    }
    QFileInfo fi(filePath);
    if (fi.exists()) {
        // Проверка (Буква "С" теперь всегда выведется корректно)
        qDebug() << "Считанный путь к файлу в main:" << filePath;
        qDebug() << "Считанный ID контекста в main:" << contextId;
    } else {
        qDebug() << "EXTRA CASES: ";
        qDebug() << filePath;
        qDebug() << contextId;
    }

    qInfo() << "###################\n";
    qInfo() << "# Forest Guard    #\n";
    qInfo() << "###################\n";
    QTranslator translator;
    translator.load("app_ru.qm");
    qApp->installTranslator(&translator);
    MainWindowSatelliteComparator w(filePath, contextId);
    w.show();
    return a.exec();
}
