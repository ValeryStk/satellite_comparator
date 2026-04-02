#ifndef SATELLITE_XML_READER_H
#define SATELLITE_XML_READER_H

#include "QString"
#include "satellites_structs.h"
#include "unordered_map"

namespace satc {

sad::LANDSAT_METADATA_FILE readLandsatXmlHeader(
    const QString& pathToLandsatHeader);

QString getPathToCloudMaskForSentinel(const QString& pathToManifestXml);

QString extractSpacecraftName(const QString& xmlFilePath);

std::unordered_map<int, double> extractSolarIrradianceForSentinel(
    const QString& filename);

double getSunZenitAngleForSentinel(const QString& filename);

double getSunAzimuthAngleForSentinel(const QString& filename);
}  // namespace satc

#endif  // SATELLITE_XML_READER_H
