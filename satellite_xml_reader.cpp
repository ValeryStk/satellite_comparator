#include "satellite_xml_reader.h"
#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include "QDebug"

namespace{
QString traverseDom(const QDomNode& node,
                    const QString& parent_name,
                    const QString& child_name) {
    QString currentName = node.isElement() ? node.toElement().tagName() : node.nodeName();
    QDomNode parent = node.parentNode();
    QString parentName = parent.isElement() ? parent.toElement().tagName() : parent.nodeName();
    if (!parent.isNull()) {
        if(parentName  == parent_name){
            if(currentName == child_name){
                return node.toElement().text();
            }
        };
    }
    QDomNode child = node.firstChild();
    QString result;
    while (!child.isNull()) {
        result = traverseDom(child,parent_name,child_name);
        if(result.isEmpty()==false)break;
        child = child.nextSibling();
    }
    return result;
}
}


namespace satc{

sad::LANDSAT_METADATA_FILE satc::readLandsatXmlHeader(const QString &pathToLandsatHeader)
{
    sad::LANDSAT_METADATA_FILE lmd;
    QFile file(pathToLandsatHeader);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл";
        return lmd;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qCritical() << "Ошибка разбора XML.";
        file.close();
        return lmd;
    }

    QDomElement root = doc.documentElement();

    //qDebug()<<traverseDom(root,"PRODUCT_CONTENTS","LANDSAT_PRODUCT_ID");
    for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
      QString result = traverseDom(root,"PRODUCT_CONTENTS",sad::landsat9_bands_keys[i]);
      qDebug()<<result;
    }


    file.close();
    return lmd;

}

}// end namespace satc
