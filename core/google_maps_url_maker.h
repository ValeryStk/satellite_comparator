#ifndef GOOGLE_MAPS_URL_MAKER_H
#define GOOGLE_MAPS_URL_MAKER_H

#include "string"

namespace maps_utility
{
 std::string makeGoogleUrl(const double &latitude, const double &longitude);
};

#endif // GOOGLE_MAPS_URL_MAKER_H
