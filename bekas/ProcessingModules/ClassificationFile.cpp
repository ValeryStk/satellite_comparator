#include "ClassificationFile.h"
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>

ClassificationFile::ClassificationFile(QObject *parent, QString jsonFilePath)
    : QObject{parent}
{
    m_jsonFilePath = jsonFilePath;
    QJsonObject jsonObject;
    if(getJsonObjectFromFile(jsonObject)){
        qDebug()<<"Classification file ok";
        m_fileList = getStructFromJsonObject(jsonObject);
        std::sort(m_fileList.begin(), m_fileList.end());
        for(int i = 0; i < m_fileList.count(); i++){
            std::sort(m_fileList[i].classDescriptions.begin(), m_fileList[i].classDescriptions.end());
        }
    }else{
        m_jsonFilePath = QDir::currentPath() + "/classes.json";
        if(getJsonObjectFromFile(jsonObject)){
            qDebug()<<"Open Restored Classification file";
            m_fileList = getStructFromJsonObject(jsonObject);
            std::sort(m_fileList.begin(), m_fileList.end());
            for(int i = 0; i < m_fileList.count(); i++){
                std::sort(m_fileList[i].classDescriptions.begin(), m_fileList[i].classDescriptions.end());
            }
        }
    }
}

QList<QString> ClassificationFile::getGeneralTypesList()
{
    QList<QString> res;
    foreach(ClassificationDescr cDescr, m_fileList){
        res.append(cDescr.generalType);
    }
    return res;
}

QList<QString> ClassificationFile::getClassesList(QString generalType)
{
    QList<QString> res;

    ClassificationDescr classification;
    classification.generalType = generalType;
    int typeIndex = getClassificationIndex<ClassificationDescr>(m_fileList, classification);

    foreach(ObjectClassDescr ocd, m_fileList.at(typeIndex).classDescriptions){
        res.append(ocd.objClassName);
    }

    return res;
}

QList<QString> ClassificationFile::getObjectNames(QString generalType, QString objectClass)
{
    QList<QString> res;

    ClassificationDescr classification;
    classification.generalType = generalType;
    int typeIndex = getClassificationIndex<ClassificationDescr>(m_fileList, classification);

    ObjectClassDescr objClassDescr;
    objClassDescr.objClassName = objectClass;
    int classIndex = getClassificationIndex<ObjectClassDescr>(m_fileList.at(typeIndex).classDescriptions,
                                                              objClassDescr);
    objClassDescr = m_fileList.at(typeIndex).classDescriptions.at(classIndex);

    res = objClassDescr.objectNames;
    return res;
}

void ClassificationFile::addGeneralType(QString genType)
{
    ClassificationDescr classification;
    classification.generalType = genType;
    m_fileList.append(classification);

    QJsonObject jsonObject = formJsonObjectFromStruct();
    saveJsonObjectToFile(jsonObject);
}

void ClassificationFile::addClass(QString genType, QString className)
{
    ClassificationDescr classification;
    classification.generalType = genType;
    int typeIndex = getClassificationIndex<ClassificationDescr>(m_fileList, classification);

    QList<ObjectClassDescr> classDecriptions = m_fileList.at(typeIndex).classDescriptions;
    ObjectClassDescr ocd;
    ocd.objClassName = className;
    classDecriptions.append(ocd);
    classification.classDescriptions.append(classDecriptions);

    m_fileList.replace(typeIndex, classification);

    QJsonObject jsonObject = formJsonObjectFromStruct();
    saveJsonObjectToFile(jsonObject);
}

void ClassificationFile::addName(QString genType, QString className, QString objectName)
{
    ClassificationDescr classification;
    classification.generalType = genType;
    int typeIndex = getClassificationIndex<ClassificationDescr>(m_fileList, classification);

    ObjectClassDescr objClassDescr;
    objClassDescr.objClassName = className;
    int classIndex = getClassificationIndex<ObjectClassDescr>(m_fileList.at(typeIndex).classDescriptions,
                                                              objClassDescr);
    objClassDescr = m_fileList.at(typeIndex).classDescriptions.at(classIndex);

    QList<QString> names = objClassDescr.objectNames;
    names.append(objectName);
    objClassDescr.objectNames = names;
    m_fileList[typeIndex].classDescriptions.replace(classIndex, objClassDescr);

    QJsonObject jsonObject = formJsonObjectFromStruct();
    saveJsonObjectToFile(jsonObject);
}

bool ClassificationFile::getJsonObjectFromFile(QJsonObject &object){

    QFile file(m_jsonFilePath);
    if(file.exists()){
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug() << "Classification file can't be opened!" <<m_jsonFilePath;
            return false;
        };
        QByteArray data = file.readAll();
        QJsonParseError errorPtr;
        object = QJsonDocument::fromJson(data, &errorPtr).object();
        if (object.isEmpty()) {
            qDebug() << "JSON IS EMPTY: "<<errorPtr.errorString();
            return false;
        }
        file.close();
        return true;
    }else{
        qDebug() << "No classification file in folder!"<<m_jsonFilePath;
        return false;
    }

}

QJsonObject ClassificationFile::formJsonObjectFromStruct()
{
    QJsonObject classificationObject;
    QJsonArray generalTypesArray;
    for(int i = 0; i < m_fileList.count(); ++i){
        QJsonObject generalType;
        generalType["gen_type"] = m_fileList.at(i).generalType;

        QJsonArray classesArray;
        for(int k = 0; k < m_fileList.at(i).classDescriptions.count(); ++k){
            QJsonObject objectClass;
            objectClass["class_name"] = m_fileList.at(i).classDescriptions.at(k).objClassName;

            QJsonArray namesArray;
            for(int l = 0; l < m_fileList.at(i).classDescriptions.at(k).objectNames.count(); ++l){
                QJsonObject objectName;
                objectName["specific_name"] = m_fileList.at(i).classDescriptions.at(k).objectNames.at(l);
                namesArray.append(objectName);
            }

            objectClass["object_names"] = namesArray;
            classesArray.append(objectClass);
        }

        generalType["classes"] = classesArray;
        generalTypesArray.append(generalType);
    }
    classificationObject["classification"] = generalTypesArray;
    return classificationObject;
}

bool ClassificationFile::saveJsonObjectToFile(QJsonObject &jsonObject)
{
    QFile file(m_jsonFilePath);
    if(!file.open(QIODevice::WriteOnly))return false;
    auto json_doc = QJsonDocument(jsonObject).toJson(QJsonDocument::JsonFormat::Indented);
    auto result = file.write(json_doc);
    file.close();
    if(result == -1)return false;
    else return true;
}

QList<ClassificationDescr> ClassificationFile::getStructFromJsonObject(const QJsonObject &json_object)
{
    QList<ClassificationDescr> fileList;
    auto classification = json_object.find("classification").value().toArray();
    for(auto &&it:classification){
        ClassificationDescr generalDescr;
        auto description = it.toObject();
        generalDescr.generalType = description.find("gen_type")->toString();

        QList<ObjectClassDescr> classesDecr;
        auto classesArray = description.find("classes").value().toArray();
        for(auto &&secIt:classesArray){
            ObjectClassDescr ocd;
            auto objClass = secIt.toObject();
            ocd.objClassName = objClass.find("class_name")->toString();

            auto namesArray = objClass.find("object_names")->toArray();
            for(auto &&namesIt:namesArray){
                auto objName = namesIt.toObject();
                ocd.objectNames.append(objName.find("specific_name")->toString());
            }

            classesDecr.append(ocd);
        }
        generalDescr.classDescriptions.append(classesDecr);

        fileList.append(generalDescr);
    }
    return fileList;
}
