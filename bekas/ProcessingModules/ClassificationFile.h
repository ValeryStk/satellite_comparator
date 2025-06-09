#ifndef CLASSIFICATIONFILE_H
#define CLASSIFICATIONFILE_H

#include <QObject>

struct ObjectClassDescr{
    QString objClassName;
    QList<QString> objectNames;

    bool operator < (const  ObjectClassDescr& ocd) const
    {
        return (objClassName < ocd.objClassName);
    }
    bool operator > (const  ObjectClassDescr& ocd) const
    {
        return (objClassName > ocd.objClassName);
    }
    bool operator == (const  ObjectClassDescr& ocd) const
    {
        return (objClassName == ocd.objClassName);
    }
};

struct ClassificationDescr{
    QString generalType;
    QList<ObjectClassDescr> classDescriptions;

    bool operator < (const  ClassificationDescr& cd) const
    {
        return (generalType < cd.generalType);
    }
    bool operator > (const  ClassificationDescr& cd) const
    {
        return (generalType > cd.generalType);
    }
    bool operator == (const  ClassificationDescr& cd) const
    {
        return (generalType == cd.generalType);
    }
};

class ClassificationFile : public QObject
{
    Q_OBJECT
public:
    explicit ClassificationFile(QObject *parent, QString jsonFilePath);

    QList<QString> getGeneralTypesList();

    QList<QString> getClassesList(QString generalType);

    QList<QString> getObjectNames(QString generalType, QString objectClass);

    void addGeneralType(QString genType);

    void addClass(QString genType, QString className);

    template <typename T>
    int getClassificationIndex(QList<T> listToSearch, T classification){
        int index = 0;
        foreach(T cDescr, listToSearch){
            if(cDescr == classification)
                break;
            index++;
        }
        return index;
    }

    void addName(QString genType, QString className, QString objectName);

signals:

private:
    bool getJsonObjectFromFile(QJsonObject &object);

    QList<ClassificationDescr> getStructFromJsonObject(const QJsonObject &json_object);

    QJsonObject formJsonObjectFromStruct();

    bool saveJsonObjectToFile(QJsonObject &jsonObject);


    QString m_jsonFilePath;
    QList<ClassificationDescr> m_fileList;
};

#endif // CLASSIFICATIONFILE_H
