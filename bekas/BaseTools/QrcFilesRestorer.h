#ifndef QRCFILESRESTORER_H
#define QRCFILESRESTORER_H
#include "qstring.h"
#include <QFile>
/**
 * @brief The QrcFilesRestorer class
 * Class needed to restore files from Application resources while there are no files in program folder
 */
class QrcFilesRestorer
{
public:
    /**
     * @brief QrcFilesRestorer  Constructor
     * @param path2Qrc  Path to qrc-file
     */
    QrcFilesRestorer(QString path2Qrc);

    /**
     * @brief restoreFilesFromQrc   Static function to restore files from resources
     * @param path2Qrc  Path to qrc-file
     */
    static void restoreFilesFromQrc(QString path2Qrc);

};

#endif // QRCFILESRESTORER_H
