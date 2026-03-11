#ifndef SATELLITES_BANDS_MAP_H
#define SATELLITES_BANDS_MAP_H

#include "array"
#include "string"
#include "unordered_map"

namespace sam {

enum class sk { SENTINEL, LANDSAT };

extern const char kAER[];
extern const char kBLUE[];
extern const char kGREEN[];
extern const char kRED[];
extern const char kRE1[];
extern const char kRE2[];
extern const char kRE3[];
extern const char kNIR1[];
extern const char kNIR2[];
extern const char kWV[];
extern const char kSWIR1[];
extern const char kSWIR2[];
extern const char kSWIR3[];

// clang-format off
enum BANDS_INDEX {
    _AER,
    _BLUE,
    _GREEN,
    _RED,
    _RE1,
    _RE2,
    _RE3,
    _NIR1,
    _NIR2,
    _WV,
    _SWIR1,
    _SWIR2,
    _SWIR3
};
// clang-format on

extern const std::unordered_map<std::string, std::unordered_map<sk, int>>
    bands_map;

extern int getBandIndex(const std::string& band_name, sk satellite);

extern int getBandIndex(const char* band_name, sk satellite);

}  // namespace sam

#endif  // SATELLITESBANDSMAP_H
