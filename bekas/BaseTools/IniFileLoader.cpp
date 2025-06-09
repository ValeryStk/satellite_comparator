#include "IniFileLoader.h"
#include <QDir>
#include <QDebug>

IniFileLoader::IniFileLoader(QObject *parent, QString projectName) : QObject(parent)
{
    m_settings = createSettingsObject(projectName);
}

IniFileLoader::~IniFileLoader()
{
    delete m_settings;
}

QSettings *IniFileLoader::settings() const
{
    return m_settings;
}

QSettings *IniFileLoader::createSettingsObject(QString projectName)
{
    QString currentPath = QDir::currentPath();
    currentPath.append("/" + projectName + ".ini");
    bool isIniExists = QFile(currentPath).exists();

    if(!isIniExists){
        QFile resFile;
        QString qrcFileName = ":/4Release/" + projectName + ".ini";
        resFile.copy(qrcFileName, currentPath);
        QFile fileCopied(currentPath);
        fileCopied.setPermissions(QFileDevice::WriteOther);
    }

    return new QSettings(currentPath,QSettings::IniFormat);
}
