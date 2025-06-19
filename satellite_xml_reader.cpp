#include "satellite_xml_reader.h"
#include <QFile>
#include <QXmlStreamReader>
#include "QDebug"


namespace satc{

sas::LANDSAT_METADATA_FILE satc::readLandsatXmlHeader(const QString &pathToLandsatHeader)
{
    sas::LANDSAT_METADATA_FILE lmd;
    QFile file(pathToLandsatHeader);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл";
        return lmd;
    }
    QXmlStreamReader xml(&file);
    QString parentTag;
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if(parentTag=="PRODUCT_CONTENTS"){
        qDebug()<<xml.name().toString();
        }
        if (xml.isStartElement()) {

        parentTag = xml.name().toString();
        }
    }

    if (xml.hasError()) {
        qDebug() << "Ошибка XML:" << xml.errorString();
    }

    file.close();
    return lmd;

}

}// end namespace satc
