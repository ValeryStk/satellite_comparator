#ifndef MATFILESOPERATOR_H
#define MATFILESOPERATOR_H
#include <QColor>
#include <QList>
#include <QString>

extern const QString matlabAppDirRelativeName;
extern const QString matlabAppExeFile;
extern const QString matFileName;

struct BecasDataFromMatlab {
    QStringList specNamesWithExtens;
    QStringList specNames;
    QString pathFolderName;
    QVector<int> selectedClustIndxs;
    QVector<QColor> colorsOfEachSpectr;
    bool isSomeErrors = false;
};

class MatFilesOperator {
public:
    MatFilesOperator();
    void saveBecasDataToMatFile(const QList<QString> &specNames,
                                const QString &pathFolderName,
                                bool isReflectance, const QString &fullMatPath);
    BecasDataFromMatlab readBecasDataFromMatlab(const QString &fullMatPath);

    //!
    //! \brief saveMultiSpecDataToMatFile
    //! \param waves - по числу спектральных каналов
    //! \param x - по числу пикселей
    //! \param y - по числу пикселей
    //! \param specs -    1-я размерность specs по числу спектров,
    //!                   2-я  - по числу каналов
    //! \param fullMatPath - путь для сохранения файла
    //!
    void saveMultiSpecDataToMatFile(const QVector<double> &waves,
                                    const QVector<int> &x,
                                    const QVector<int> &y,
                                    const QVector<QVector<double>> &specs,
                                    const QString &fullMatPath);
};

#endif  // MATFILESOPERATOR_H
