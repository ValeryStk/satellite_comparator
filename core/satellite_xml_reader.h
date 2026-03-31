#ifndef SATELLITE_XML_READER_H
#define SATELLITE_XML_READER_H

#include "QString"
#include "satellites_structs.h"

namespace satc {

sad::LANDSAT_METADATA_FILE readLandsatXmlHeader(
    const QString& pathToLandsatHeader);

QString getPathToCloudMaskForSentinel(const QString& pathToManifestXml);

QString extractSpacecraftName(const QString& xmlFilePath);

}  // namespace satc

#endif  // SATELLITE_XML_READER_H
