#include "SpectrDataSaver.h"
#include <QTextStream>
#include <QIODevice>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <qdebug.h>

SpectrDataSaver::SpectrDataSaver(QObject *parent) : QObject(parent)
{
    m_counter4Recording = 0;
    m_singleFileName = "";
}

void SpectrDataSaver::createFileAndWrite(QString absoluteFileName, QString *strData)
{
    QFile file(absoluteFileName);
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    out << *strData;
    file.close();
}

QString SpectrDataSaver::savingDirectory() const
{
    return m_savingDirectory;
}

void SpectrDataSaver::setSavingDirectory(QString savingDir)
{
    m_savingDirectory = savingDir;
}

const QString &SpectrDataSaver::singleFileName() const
{
    return m_singleFileName;
}

QString SpectrDataSaver::getStringSpectrumFromVectors(QVector<double> waves, QVector<double> values)
{
    QString specStr;
    if(waves.count() == values.count()){
        for(int i = 0; i < waves.count(); i++){
            specStr.append(QString::number(waves.at(i), 'f', 2) + "\t" + QString::number(values.at(i), 'f', 4) + "\n");
        }
    }
    return specStr;
}

void SpectrDataSaver::checkDirAndCreateIfNeeded(QString folderPath)
{
    QDir checkDir(folderPath);
    if(!checkDir.exists()){
        checkDir.mkdir(folderPath);
    }
}

void SpectrDataSaver::setSingleFileName(const QString &singleFileName)
{
    m_singleFileName = singleFileName;
}

void SpectrDataSaver::saveStrInSeparateThread(QString baseFileName, QString strData)
{
    emit sendTextMessage("Сохранение файла спектра: " + m_savingDirectory + "/" + baseFileName + ".txt");
    runStrFileSaving(baseFileName,m_savingDirectory,strData);
}

void SpectrDataSaver::saveStrInSeparateThread (QString strData)
{
    emit sendTextMessage("Сохранение файла спектра: " + m_singleFileName);
    runStrFileSaving(m_singleFileName,strData);
}

void SpectrDataSaver::runStrFileSaving(QString baseFileName, QString savingDir, QString strData)
{
    checkDirAndCreateIfNeeded(savingDir);
    QString absoluteFileName = savingDir + "/" + baseFileName + ".txt";
    createFileAndWrite(absoluteFileName, &strData);
}

void SpectrDataSaver::runStrFileSaving(QString absoluteFileName, QString strData)
{
    createFileAndWrite(absoluteFileName, &strData);
}

void SpectrDataSaver::setCounter4Recording(int value)
{
    m_counter4Recording = value;
}
