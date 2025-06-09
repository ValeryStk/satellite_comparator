#ifndef SPECTRDATASAVER_H
#define SPECTRDATASAVER_H

#include <QObject>
#include <QVector>
#include <QVariant>

class SpectrDataSaver : public QObject
{
    Q_OBJECT
public:
    explicit SpectrDataSaver(QObject *parent = nullptr);

    void setCounter4Recording(int value);

    QString savingDirectory() const;

    void setSingleFileName(const QString &singleFileName);

    const QString &singleFileName() const;

    static QString getStringSpectrumFromVectors(QVector<double> waves, QVector<double> values);

public slots:
    void saveStrInSeparateThread(QString baseFileName, QString strData);

    void saveStrInSeparateThread(QString strData);

signals:
    void spectrWasSaved(int);

    void spectrSeriesWasFinished();

    void sendTextMessage(QString textToShow);

public slots:
    void setSavingDirectory(QString savingDir);

private:
    static void runStrFileSaving(QString baseFileName, QString savingDir, QString strData);

    static void runStrFileSaving(QString absoluteFileName, QString strData);

    static void createFileAndWrite(QString absoluteFileName, QString *strData);

    static void checkDirAndCreateIfNeeded(QString folderPath);


    int m_counter4Recording;        //!< Счетчик для прогресс-баров
    QString m_savingDirectory;      //!< Общая директория для записи
    QString m_singleFileName;       //!< Имя файла в случае сохранения не по пути по умолчанию

    QVector <double> m_bands;           //!< Каналы
};

#endif // SPECTRDATASAVER_H
