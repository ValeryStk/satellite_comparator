#include "satellites_bands_map.h"

#include "array"
#include "string"
#include "unordered_map"

namespace sam {

// clang-format off
    const char kAER[]   = "AER";
    const char kBLUE[]  = "BLUE";
    const char kGREEN[] = "GREEN";
    const char kRED[]   = "RED";
    const char kRE1[]   = "RE1";
    const char kRE2[]   = "RE2";
    const char kRE3[]   = "RE3";
    const char kNIR1[]  = "NIR1";
    const char kNIR2[]  = "NIR2";
    const char kWV[]    = "WV";
    const char kSWIR1[]  = "SWIR";
    const char kSWIR2[] = "SWIR1";
    const char kSWIR3[] = "SWIR2";

const std::unordered_map<std::string, std::unordered_map<sk, int>> bands_map = {
    {std::string(kAER),   {{sk::SENTINEL, 0},  {sk::LANDSAT, 0}}},
    {std::string(kBLUE),  {{sk::SENTINEL, 1},  {sk::LANDSAT, 1}}},
    {std::string(kGREEN), {{sk::SENTINEL, 2},  {sk::LANDSAT, 2}}},
    {std::string(kRED),   {{sk::SENTINEL, 3},  {sk::LANDSAT, 3}}},
    {std::string(kRE1),   {{sk::SENTINEL, 4},  {sk::LANDSAT, -1}}},
    {std::string(kRE2),   {{sk::SENTINEL, 5},  {sk::LANDSAT, -1}}},
    {std::string(kRE3),   {{sk::SENTINEL, 6},  {sk::LANDSAT, -1}}},
    {std::string(kNIR1),  {{sk::SENTINEL, 7},  {sk::LANDSAT, 4}}},
    {std::string(kNIR2),  {{sk::SENTINEL, 8},  {sk::LANDSAT, 4}}},
    {std::string(kWV),    {{sk::SENTINEL, 9}, {sk::LANDSAT, -1}}},
    {std::string(kSWIR1),  {{sk::SENTINEL, 10}, {sk::LANDSAT, 8}}},
    {std::string(kSWIR2), {{sk::SENTINEL, 11}, {sk::LANDSAT, 5}}},
    {std::string(kSWIR3), {{sk::SENTINEL, 12}, {sk::LANDSAT, 6}}},
};
// clang-format on

int getBandIndex(const std::string &band_name, sk satellite) {
    auto outer_it = bands_map.find(band_name);
    if (outer_it == bands_map.end()) {
        return -1;  // Канал не найден
    }

    auto inner_it = outer_it->second.find(satellite);
    if (inner_it == outer_it->second.end()) {
        return -1;  // Спутник не поддерживает канал
    }

    return inner_it->second;  // Индекс канала (>=0) или -1 (не поддерживается)
}

int getBandIndex(const char *band_name, sk satellite) {
    return getBandIndex(std::string(band_name), satellite);
}

}  // end namespace sam
