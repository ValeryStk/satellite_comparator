#ifndef FILESPARSER_H
#define FILESPARSER_H

#include <QObject>
#include <bekas/BaseTools/DBJson.h>

class FilesParser : public QObject
{
    Q_OBJECT
public:
    explicit FilesParser(QObject *parent, QString inputDirPath);

    void setInputDirPath(const QString &newInputDirPath);

    void parseInputDir();

    void saveCurrentSpectrum();

    db_json::META_DATA getCurrentMetadata();

    QVector<double> getRflSpectrumValues(double &maxValue);

    QVector<double> getBrightPattSpectrumValues(double &maxValue);

    QVector<double> getBrightTemplSpectrumValues(double &maxValue);

    QVector<double> getRflWaves();

    QVector<double> getBrightPattWaves();

    QVector<double> getBrightTemplWaves();

    QString getImagePath(QVector<QPair<int,int>> &spFov);

    const QStringList &spectraNamesList() const;

    void setCurrentSpectrumIndex(int newCurrentSpectrumIndex);

    void removeDataAtIndex(int dataIndexInList);

    const db_json::SPECTRAL_STRUCT &spectrum() const;

    const QString &inputDirPath() const;

    const QString currentSpName() const;

    void setClassification(db_json::CLASSIFICATION classification);

    const db_json::CLASSIFICATION &currClassification() const;

    QString getInputDirPathWithSlash() const;

    QStringList getSpectraNamesWithExtensionList() const;

signals:
    void sendMessage(QString text);

private:
    static QList<QString> prepareEntryList(QString pathToDir);
    void parseCurrentSpectrum();

    QString m_inputDirPath;
    int m_currentSpectrumIndex;
    QList<QString> m_spectraPathesList;

    QStringList m_spectraNamesList;
    QStringList m_spectraNamesWithExtensionList;
    db_json::SPECTRAL_STRUCT m_spectrum;
    db_json::CLASSIFICATION m_currClassification;
};

#endif // FILESPARSER_H
