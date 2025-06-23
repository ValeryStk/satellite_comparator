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


    // READING PROJECTION_ATTRIBUTES FOR LANDSAT FROM XML

    result = traverseDom(root,"PROJECTION_ATTRIBUTES","UTM_ZONE");
    lmd.projection_attributes.utm_zone = result;

    result = traverseDom(root,"PROJECTION_ATTRIBUTES","GRID_CELL_SIZE_REFLECTIVE");
    lmd.projection_attributes.grid_cell_size_reflective = result;

    result = traverseDom(root,"PROJECTION_ATTRIBUTES","ORIENTATION");
    lmd.projection_attributes.orientation = result;

    result = traverseDom(root,"PROJECTION_ATTRIBUTES","CORNER_UL_PROJECTION_X_PRODUCT");
    lmd.projection_attributes.corner_ul_projection_x_product = result;

    result = traverseDom(root,"PROJECTION_ATTRIBUTES","CORNER_UL_PROJECTION_Y_PRODUCT");
    lmd.projection_attributes.corner_ul_projection_y_product = result;


    // READING LEVEL2_SURFACE_REFLECTANCE_PARAMETERS FOR LANDSAT FROM XML
    for(int i=0;i<LANDSAT_9_BANDS_NUMBER;++i){
      QString add_result = traverseDom(root,"LEVEL2_SURFACE_REFLECTANCE_PARAMETERS",sad::landsat9_add_reflectence_keys[i]);
      QString mult_result = traverseDom(root,"LEVEL2_SURFACE_REFLECTANCE_PARAMETERS",sad::landsat9_mult_reflectence_keys[i]);
      lmd.level2_surface_reflectance_parameters.reflectance_add_band[i] = add_result;
      lmd.level2_surface_reflectance_parameters.reflectance_mult_band[i] = mult_result;
    }
    lmd.isHeaderValid = true;
    file.close();
    return lmd;

}

}// end namespace satc
