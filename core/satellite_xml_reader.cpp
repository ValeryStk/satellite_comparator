#include "satellite_xml_reader.h"

#include <QDomDocument>
#include <QDomNode>
#include <QFile>
#include <QXmlStreamReader>

#include "QDebug"

namespace {
QString traverseDom(const QDomNode& node, const QString& parent_name,
                    const QString& child_name) {
    QString currentName =
        node.isElement() ? node.toElement().tagName() : node.nodeName();
    QDomNode parent = node.parentNode();
    QString parentName =
        parent.isElement() ? parent.toElement().tagName() : parent.nodeName();
    if (!parent.isNull()) {
        if (parentName == parent_name) {
            if (currentName == child_name) {
                return node.toElement().text();
            }
        };
    }
    QDomNode child = node.firstChild();
    QString result;
    while (!child.isNull()) {
        result = traverseDom(child, parent_name, child_name);
        if (result.isEmpty() == false) break;
        child = child.nextSibling();
    }
    return result;
}

double extractDoubleParameter(const QString& filename, const QString& tagName) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << file.errorString();
        return -1.0;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Invalid XML data";
        file.close();
        return -1.0;
    }
    file.close();

    QDomNodeList nodes = doc.elementsByTagName(tagName);
    if (!nodes.isEmpty()) {
        QDomElement e = nodes.item(0).toElement();
        return e.text().toDouble();
    }

    return -1.0;
}

}  // namespace

namespace satc {

sad::LANDSAT_METADATA_FILE readLandsatXmlHeader(
    const QString& pathToLandsatHeader) {
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
    result = traverseDom(root, "PRODUCT_CONTENTS", "LANDSAT_PRODUCT_ID");
    lmd.product_contents.landsat_product_id = result;

    result = traverseDom(root, "PRODUCT_CONTENTS", "PROCESSING_LEVEL");
    lmd.product_contents.processing_level = result;

    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        QString result =
            traverseDom(root, "PRODUCT_CONTENTS", sad::landsat9_bands_keys[i]);
        if (result.isEmpty()) {
            lmd.landsat9_missed_channels[i] = true;
        } else {
            lmd.landsat9_missed_channels[i] = false;
        }
        lmd.product_contents.file_name_bands[i] = result;
        qDebug() << result;
    }

    // READING IMAGE_ATTRIBUTES FOR LANDSAT FROM XML
    result = traverseDom(root, "IMAGE_ATTRIBUTES", "SPACECRAFT_ID");
    lmd.image_attributes.spacecraft_id = result;

    result = traverseDom(root, "IMAGE_ATTRIBUTES", "SENSOR_ID");
    lmd.image_attributes.sensor_id = result;

    result = traverseDom(root, "IMAGE_ATTRIBUTES", "DATE_ACQUIRED");
    lmd.image_attributes.date_acquired = result;

    result = traverseDom(root, "IMAGE_ATTRIBUTES", "SUN_AZIMUTH");
    lmd.image_attributes.sun_azimuth = result;

    result = traverseDom(root, "IMAGE_ATTRIBUTES", "SUN_ELEVATION");
    lmd.image_attributes.sun_elevation = result;

    // READING PROJECTION_ATTRIBUTES FOR LANDSAT FROM XML

    result = traverseDom(root, "PROJECTION_ATTRIBUTES", "UTM_ZONE");
    lmd.projection_attributes.utm_zone = result;

    result =
        traverseDom(root, "PROJECTION_ATTRIBUTES", "GRID_CELL_SIZE_REFLECTIVE");
    lmd.projection_attributes.grid_cell_size_reflective = result;

    result = traverseDom(root, "PROJECTION_ATTRIBUTES", "ORIENTATION");
    lmd.projection_attributes.orientation = result;

    result = traverseDom(root, "PROJECTION_ATTRIBUTES",
                         "CORNER_UL_PROJECTION_X_PRODUCT");
    lmd.projection_attributes.corner_ul_projection_x_product = result;

    result = traverseDom(root, "PROJECTION_ATTRIBUTES",
                         "CORNER_UL_PROJECTION_Y_PRODUCT");
    lmd.projection_attributes.corner_ul_projection_y_product = result;

    // READING LEVEL2_SURFACE_REFLECTANCE_PARAMETERS FOR LANDSAT FROM XML
    for (int i = 0; i < LANDSAT_BANDS_NUMBER; ++i) {
        QString add_result =
            traverseDom(root, "LEVEL2_SURFACE_REFLECTANCE_PARAMETERS",
                        sad::landsat9_add_reflectence_keys[i]);
        QString mult_result =
            traverseDom(root, "LEVEL2_SURFACE_REFLECTANCE_PARAMETERS",
                        sad::landsat9_mult_reflectence_keys[i]);
        lmd.level2_surface_reflectance_parameters.reflectance_add_band[i] =
            add_result;
        lmd.level2_surface_reflectance_parameters.reflectance_mult_band[i] =
            mult_result;
    }
    lmd.isHeaderValid = true;
    file.close();
    return lmd;
}

QString getPathToCloudMaskForSentinel(const QString& xmlFilePath) {
    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << xmlFilePath;
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Ошибка парсинга XML";
        file.close();
        return {};
    }
    file.close();

    // Ищем dataObject с ID="CloudPrbMask_Tile1_Data"
    QDomNodeList dataObjects = doc.elementsByTagName("dataObject");
    for (int i = 0; i < dataObjects.size(); ++i) {
        QDomElement dataObj = dataObjects.at(i).toElement();
        if (dataObj.attribute("ID") == "CloudPrbMask_Tile1_Data") {
            QDomNodeList fileLocations =
                dataObj.elementsByTagName("fileLocation");
            if (!fileLocations.isEmpty()) {
                QString href =
                    fileLocations.at(0).toElement().attribute("href");
                if (!href.isEmpty()) {
                    return href;  // Например:
                                  // "./GRANULE/.../MSK_CLDPRB_20m.jp2"
                }
            }
        }
    }

    qWarning() << "Путь к MSK_CLDPRB_20m.jp2 не найден в XML";
    return {};
}

QString extractSpacecraftName(const QString& xmlFilePath) {
    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << xmlFilePath;
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        // qWarning() << "Ошибка парсинга XML:" << doc.err;
        file.close();
        return {};
    }
    file.close();

    // Ищем dataObject с spacecraftName или platform
    QDomNodeList dataObjects = doc.elementsByTagName("dataObject");
    for (int i = 0; i < dataObjects.size(); ++i) {
        QDomElement dataObj = dataObjects.at(i).toElement();
        QDomNodeList spacecraftTags =
            dataObj.elementsByTagName("SPACECRAFT_NAME");
        if (!spacecraftTags.isEmpty()) {
            QDomNode spacecraftNode = spacecraftTags.at(0);
            QString name = spacecraftNode.firstChild().nodeValue().trimmed();
            if (!name.isEmpty()) {
                return name;  // "Sentinel-2A"
            }
        }

        // Альтернатива: метаданные в <platform> или <mission>
        QDomNodeList platforms = dataObj.elementsByTagName("platform");
        for (int j = 0; j < platforms.size(); ++j) {
            QDomNodeList shortNames =
                platforms.at(j).toElement().elementsByTagName("shortName");
            if (!shortNames.isEmpty()) {
                return shortNames.at(0).firstChild().nodeValue().trimmed();
            }
        }
    }

    // Прямой поиск в корне документа (standalone тег)
    QDomNodeList spacecraftRoot = doc.elementsByTagName("SPACECRAFT_NAME");
    if (!spacecraftRoot.isEmpty()) {
        return spacecraftRoot.at(0).firstChild().nodeValue().trimmed();
    }

    qWarning() << "SPACECRAFT_NAME не найден в XML";
    return {};
}

std::unordered_map<int, double> extractSolarIrradianceForSentinel(
    const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << file.errorString();
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Invalid XML data";
        file.close();
        return {};
    }
    file.close();

    std::unordered_map<int, double> solar_irradiance;

    QDomElement root = doc.documentElement();
    QDomNodeList nodes = root.elementsByTagName("SOLAR_IRRADIANCE");

    for (int i = 0; i < nodes.size(); ++i) {
        QDomElement e = nodes.item(i).toElement();
        int bandId = e.attribute("bandId").toInt();
        double value = e.text().toDouble();
        solar_irradiance[bandId] = value;
    }

    return solar_irradiance;
}

double getSunZenitAngleForSentinel(const QString& filename) {
    return extractDoubleParameter(filename, "ZENITH_ANGLE");
}

double getSunAzimuthAngleForSentinel(const QString& filename) {
    return extractDoubleParameter(filename, "AZIMUTH_ANGLE");
}

double getAverageCaptureAngle(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filename;
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        // qWarning() << "Ошибка парсинга XML:" << doc.err;
        file.close();
        return {};
    }
    file.close();

    QDomNodeList nodes = doc.elementsByTagName("Mean_Viewing_Incidence_Angle");
    if (nodes.isEmpty()) {
        qDebug() << "Mean_Viewing_Incidence_Angle not founded...";
        return 0.0;
    }

    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < nodes.count(); ++i) {
        QDomElement bandEl = nodes.at(i).toElement();
        QDomElement zenithEl = bandEl.firstChildElement("ZENITH_ANGLE");
        if (!zenithEl.isNull()) {
            sum += zenithEl.text().toDouble();
            ++count;
        }
    }

    return count > 0 ? sum / count : 0.0;
}

}  // end namespace satc
