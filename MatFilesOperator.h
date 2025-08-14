#ifndef MATFILESOPERATOR_H
#define MATFILESOPERATOR_H
#include <QString>
#include <QList>
#include <QColor>

struct BecasDataFromMatlab
{
    QStringList specNamesWithExtens;
    QStringList specNames;
    QString pathFolderName;
    QVector<int> selectedClustIndxs;
    QVector<QColor> colorsOfEachSpectr;
    bool isSomeErrors = false;
};

class MatFilesOperator
{
public:
    MatFilesOperator();
    void saveBecasData(const QList<QString> &specNames,
                       const QString &pathFolderName, bool isReflectance,
                       const QString &fullMatPath);
    BecasDataFromMatlab readBecasDataFromMatlab(const QString &fullMatPath);
};

#endif // MATFILESOPERATOR_H
