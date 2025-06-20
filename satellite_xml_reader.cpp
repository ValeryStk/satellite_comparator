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

sad::LANDSAT_METADATA_FILE readLandsatXmlHeader(const QString &pathToLandsatHeader)
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
    QString result;

    // READING PRODUCT_CONTENTS FOR LANDSAT FROM XML
    result = traverseDom(root,"PRODUCT_CONTENTS","LANDSAT_PRODUCT_ID");
    lmd.product_contents.landsat_product_id = result;

    result = traverseDom(root,"PRODUCT_CONTENTS","PROCESSING_LEVEL");
    lmd.product_contents.processing_level = result;

    for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
      QString result = traverseDom(root,"PRODUCT_CONTENTS",sad::landsat9_bands_keys[i]);
      if(result.isEmpty()){
          lmd.landsat9_missed_channels[i]=true;
      }
      else{
          lmd.landsat9_missed_channels[i]=false;
      }
      lmd.product_contents.file_name_bands[i] = result;
      qDebug()<<result;
    }

    // READING IMAGE_ATTRIBUTES FOR LANDSAT FROM XML
    result = traverseDom(root,"IMAGE_ATTRIBUTES","SPACECRAFT_ID");
    lmd.image_attributes.spacecraft_id = result;

    result = traverseDom(root,"IMAGE_ATTRIBUTES","SENSOR_ID");
    lmd.image_attributes.sensor_id = result;

    result = traverseDom(root,"IMAGE_ATTRIBUTES","DATE_ACQUIRED");
    lmd.image_attributes.date_acquired = result;

    result = traverseDom(root,"IMAGE_ATTRIBUTES","SUN_AZIMUTH");
    lmd.image_attributes.sun_azimuth = result;

    result = traverseDom(root,"IMAGE_ATTRIBUTES","SUN_ELEVATION");
    lmd.image_attributes.sun_elevation = result;


    file.close();
    return lmd;

}

}// end namespace satc
