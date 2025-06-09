#include "FilesParser.h"
#include <QDir>
#include <QJsonObject>

FilesParser::FilesParser(QObject *parent, QString inputDirPath)
    : QObject{parent}
{
    m_inputDirPath = inputDirPath;
    m_currentSpectrumIndex = 0;
}

void FilesParser::setInputDirPath(const QString &newInputDirPath)
{
    m_inputDirPath = newInputDirPath;
}

void FilesParser::parseInputDir()
{
    m_spectraPathesList.clear();
    m_spectraNamesList.clear();
    QDir parsingDir(m_inputDirPath);
    if(parsingDir.exists()){
        QStringList fileNamesList = prepareEntryList(m_inputDirPath);
        if(fileNamesList.count() > 2){
            foreach(QString fileName, fileNamesList){
                if(fileName.endsWith(".json", Qt::CaseInsensitive)){
                    m_spectraPathesList.append(m_inputDirPath + "/" + fileName);
                    m_spectraNamesList.append(fileName.split(".").first());
                }
            }
            parseCurrentSpectrum();
        }
    }else{
        emit sendMessage("Неверный путь к папке");
    }
}

void FilesParser::saveCurrentSpectrum()
{
    db_json::saveStructToJsonFile(m_spectraPathesList.at(m_currentSpectrumIndex),
                                  m_spectrum,
                                  QJsonDocument::JsonFormat::Indented);
}

db_json::META_DATA FilesParser::getCurrentMetadata()
{
    return m_spectrum.md;
}

QVector<double> FilesParser::getRflSpectrumValues(double &maxValue)
{
    QVector<double> spectrum = m_spectrum.sd.values.first();

    maxValue = 0;
    foreach(double value, spectrum){
        if(value > maxValue)
            maxValue = value;
    }

    return spectrum;
}

QVector<double> FilesParser::getBrightPattSpectrumValues(double &maxValue)
{
    QVector<double> spectrum = m_spectrum.sd.values.at(1);

    maxValue = 0;
    foreach(double value, spectrum){
        if(value > maxValue)
            maxValue = value;
    }

    return spectrum;
}

QVector<double> FilesParser::getBrightTemplSpectrumValues(double &maxValue)
{
    QVector<double> spectrum = m_spectrum.sd.values.at(2);

    maxValue = 0;
    foreach(double value, spectrum){
        if(value > maxValue)
            maxValue = value;
    }

    return spectrum;
}

QVector<double> FilesParser::getRflWaves()
{
    return m_spectrum.sd.waves.at(0);
}

QVector<double> FilesParser::getBrightPattWaves()
{
    return m_spectrum.sd.waves.at(1);
}

QVector<double> FilesParser::getBrightTemplWaves()
{
    return m_spectrum.sd.waves.at(2);
}

QString FilesParser::getImagePath(QVector<QPair<int, int> > &spFov)
{
    QString path = m_inputDirPath + "/" + m_spectrum.md.images.first().pathToFile.split(".").first() + ".png";
    spFov = m_spectrum.md.images.first().spFov;
    return path;
}

QList<QString> FilesParser::prepareEntryList(QString pathToDir)
{
    QDir inputDir(pathToDir);
    QList<QString> resList = inputDir.entryList();
    if(resList.count() > 1){
        resList.removeFirst();
        resList.removeFirst();
    }
    return resList;
}

void FilesParser::parseCurrentSpectrum()
{
    if(m_currentSpectrumIndex < m_spectraPathesList.count()){
        QString spectrumPath = m_spectraPathesList.at(m_currentSpectrumIndex);
        QJsonObject object;
        if(db_json::getJsonObjectFromFile(spectrumPath, object)){
            m_spectrum = db_json::getStructFromJsonObject(object);
            //qDebug()<<"spectrum data: "<<m_spectrum.sd.values;
        }else{
            emit sendMessage("Невозможно открыть файл спектра:\n" + spectrumPath);
        }
    }
}

const db_json::CLASSIFICATION &FilesParser::currClassification() const
{
    return m_spectrum.md.classification;
}

const QString FilesParser::currentSpName() const
{
    QString spName = m_spectrum.md.date_time.toString("dd.MM.yyyy hh:mm:ss.zzz");
    return spName;
}

void FilesParser::setClassification(db_json::CLASSIFICATION classification)
{
    m_spectrum.md.classification = classification;
}

const QString &FilesParser::inputDirPath() const
{
    return m_inputDirPath;
}

const db_json::SPECTRAL_STRUCT &FilesParser::spectrum() const
{
    return m_spectrum;
}

void FilesParser::setCurrentSpectrumIndex(int newCurrentSpectrumIndex)
{
    m_currentSpectrumIndex = newCurrentSpectrumIndex;
    parseCurrentSpectrum();
}

void FilesParser::removeDataAtIndex(int dataIndexInList)
{
    QString spectrumPath = m_spectraPathesList.at(dataIndexInList);
    QString imagePath;
    QJsonObject object;
    if(db_json::getJsonObjectFromFile(spectrumPath, object)){
        db_json::SPECTRAL_STRUCT spectrum = db_json::getStructFromJsonObject(object);
        imagePath = m_inputDirPath + "/" + spectrum.md.images.first().pathToFile;
    }else{
        emit sendMessage("Невозможно открыть файл спектра:\n" + spectrumPath);
    }
    QFile::remove(spectrumPath);
    QFile::remove(imagePath);
    m_spectraPathesList.removeAt(dataIndexInList);
}

const QStringList &FilesParser::spectraNamesList() const
{
    return m_spectraNamesList;
}
