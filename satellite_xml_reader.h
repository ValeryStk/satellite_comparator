#ifndef SATELLITE_XML_READER_H
#define SATELLITE_XML_READER_H

#include "satellites_structs.h"
#include "QString"

namespace satc{

sas::LANDSAT_METADATA_FILE readLandsatXmlHeader(const QString& pathToLandsatHeader);

}


#endif // SATELLITE_XML_READER_H
