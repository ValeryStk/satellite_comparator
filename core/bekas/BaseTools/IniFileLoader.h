#ifndef INIFILELOADER_H
#define INIFILELOADER_H

#include <QObject>
#include <QSettings>

/**
 * @brief The IniFileLoader class
 * Class for INI file loading and initialisaton of Qsettings object
 */
class IniFileLoader : public QObject
{
    Q_OBJECT

    QSettings *m_settings;   //!< Settings object

public:
    /**
     * @brief IniFileLoader Constructor
     * @param parent    Parent object
     * @param projectName   Project Text Name
     */
    explicit IniFileLoader(QObject *parent = nullptr, QString projectName = "");
    ~IniFileLoader();

    /**
     * @brief settings  Function to get Settings object
     * @return Settings object
     */
    QSettings *settings() const;

    /**
     * @brief createSettingsObject  Static function to create settings object depending on project name
     * @param projectName   Project text name
     * @return  Settings object
     */
    static QSettings *createSettingsObject(QString projectName);
};

#endif // INIFILELOADER_H
