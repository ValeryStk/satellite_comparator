#include "satellites_structs.h"

namespace sad {

// clang-format off
const QString landsat9_bands_keys[LANDSAT_BANDS_NUMBER] = {
    "FILE_NAME_BAND_1",
    "FILE_NAME_BAND_2",
    "FILE_NAME_BAND_3",
    "FILE_NAME_BAND_4",
    "FILE_NAME_BAND_5",
    "FILE_NAME_BAND_6",
    "FILE_NAME_BAND_7",
    "FILE_NAME_BAND_8",
    "FILE_NAME_BAND_9",
    "FILE_NAME_BAND_10",
    "FILE_NAME_BAND_11"
};


const QString landsat9_mult_radiance_keys[LANDSAT_BANDS_NUMBER] = {
    "RADIANCE_MULT_BAND_1",
    "RADIANCE_MULT_BAND_2",
    "RADIANCE_MULT_BAND_3",
    "RADIANCE_MULT_BAND_4",
    "RADIANCE_MULT_BAND_5",
    "RADIANCE_MULT_BAND_6",
    "RADIANCE_MULT_BAND_7",
    "RADIANCE_MULT_BAND_8",
    "RADIANCE_MULT_BAND_9",
    "RADIANCE_MULT_BAND_10",
    "RADIANCE_MULT_BAND_11"
};

const QString landsat9_add_radiance_keys[LANDSAT_BANDS_NUMBER] = {
    "RADIANCE_ADD_BAND_1",
    "RADIANCE_ADD_BAND_2",
    "RADIANCE_ADD_BAND_3",
    "RADIANCE_ADD_BAND_4",
    "RADIANCE_ADD_BAND_5",
    "RADIANCE_ADD_BAND_6",
    "RADIANCE_ADD_BAND_7",
    "RADIANCE_ADD_BAND_8",
    "RADIANCE_ADD_BAND_9",
    "RADIANCE_ADD_BAND_10",
    "RADIANCE_ADD_BAND_11"
};

const QString landsat9_mult_reflectence_keys[LANDSAT_BANDS_NUMBER] = {
    "REFLECTANCE_MULT_BAND_1",
    "REFLECTANCE_MULT_BAND_2",
    "REFLECTANCE_MULT_BAND_3",
    "REFLECTANCE_MULT_BAND_4",
    "REFLECTANCE_MULT_BAND_5",
    "REFLECTANCE_MULT_BAND_6",
    "REFLECTANCE_MULT_BAND_7",
    "REFLECTANCE_MULT_BAND_8",
    "REFLECTANCE_MULT_BAND_9",
    "REFLECTANCE_MULT_BAND_10",
    "REFLECTANCE_MULT_BAND_11"
};

const QString landsat9_add_reflectence_keys[LANDSAT_BANDS_NUMBER] = {
    "REFLECTANCE_ADD_BAND_1",
    "REFLECTANCE_ADD_BAND_2",
    "REFLECTANCE_ADD_BAND_3",
    "REFLECTANCE_ADD_BAND_4",
    "REFLECTANCE_ADD_BAND_5",
    "REFLECTANCE_ADD_BAND_6",
    "REFLECTANCE_ADD_BAND_7",
    "REFLECTANCE_ADD_BAND_8",
    "REFLECTANCE_ADD_BAND_9",
    "REFLECTANCE_ADD_BAND_10",
    "REFLECTANCE_ADD_BAND_11"
};

const QString landsat_bands_gui_names[LANDSAT_BANDS_NUMBER] = {
    "B01 - 443   nm AER",
    "B02 - 482   nm BLUE",
    "B03 - 562   nm GREEN",
    "B04 - 655   nm RED",
    "B05 - 865   nm NIR1",
    "B06 - 1610  nm SWIR2",
    "B07 - 2200  nm SWIR3",
    "B08 - 590   nm PAN",
    "B09 - 1375  nm SWIR1",
    "B10 - 10800 nm TIRS1",
    "B11 - 12000 nm TIRS2"
};

const double landsat_central_wavelengths[LANDSAT_BANDS_NUMBER] = {
    443,  //0
    482,  //1
    562,  //2
    655,  //3
    865,  //4
    1610, //5
    2200, //6
    590,  //7
    1375, //8
    10800,//9
    12000 //10
};


const int sorted_landsat_bands_order_by_wavelength[LANDSAT_BANDS_NUMBER] = {
    0,//1  - 443
    1,//2  - 482
    2,//3  - 562
    7,//4  - 590
    3,//5  - 655
    4,//6  - 865
    8,//7  - 1375
    5,//8  - 1610
    6,//9  - 2200
    9,//10 - 10800
    10//11 - 12000
};


// SENTINEL-2A
const QString sentinel_2A_gui_band_names[SENTINEL_BANDS_NUMBER] = {
    "B01 - 443  nm AER",
    "B02 - 493  nm BLUE",
    "B03 - 560  nm GREEN",
    "B04 - 665  nm RED",
    "B05 - 704  nm RE1",
    "B06 - 740  nm RE2",
    "B07 - 783  nm RE3",
    "B08 - 833  nm NIR1",
    "B8A - 865  nm NIR2",
    "B09 - 945  nm WV",
    "B10 - 1374 nm SWIR1",
    "B11 - 1614 nm SWIR2",
    "B12 - 2202 nm SWIR3"
};

const double sentinel_2A_central_wave_lengths[SENTINEL_BANDS_NUMBER] = {
    443,
    493,
    560,
    665,
    704,
    740,
    783,
    833,
    865,
    945,
    1374,
    1614,
    2202
};

// SENTINEL-2B
const QString sentinel_2B_gui_band_names[SENTINEL_BANDS_NUMBER] = {
    "B01 - 442  nm AER",
    "B02 - 492  nm BLUE",
    "B03 - 559  nm GREEN",
    "B04 - 665  nm RED",
    "B05 - 704  nm RE1",
    "B06 - 739  nm RE2",
    "B07 - 780  nm RE3",
    "B08 - 833  nm NIR1",
    "B8A - 864  nm NIR2",
    "B09 - 943  nm WV",
    "B10 - 1377 nm SWIR1",
    "B11 - 1610 nm SWIR2",
    "B12 - 2186 nm SWIR3"
};

const double sentinel_2B_central_wave_lengths[SENTINEL_BANDS_NUMBER] = {
    442,
    492,
    559,
    665,
    704,
    739,
    780,
    833,
    864,
    943,
    1377,
    1610,
    2186
};

// COMMON SENTINEL
const QString sentinel_bands_keys[SENTINEL_BANDS_NUMBER] = {
    "B01",
    "B02",
    "B03",
    "B04",
    "B05",
    "B06",
    "B07",
    "B08",
    "B8A",
    "B09",
    "B10",
    "B11",
    "B12"
};


const QHash<const QString,QPair<int,int>> sentinel_resolutions = {
    {"R10m",{10980,10980}},
    {"R20m",{5490,5490}},
    {"R60m",{1830,1830}}
};

const QHash<int, QString> sentinel_resolution_by_index = {
{0,"R60m"},
{1,"R10m"},
{2,"R10m"},
{3,"R10m"},
{4,"R20m"},
{5,"R20m"},
{6,"R20m"},
{7,"R10m"},
{8,"R20m"},
{9,"R60m"},
{10,"R60m"},
{11,"R20m"},
{12,"R20m"}
};


const QHash<int, int> sentinel2A_index_by_centralWave = {
{443, 0},
{493, 1},
{560, 2},
{665, 3},
{704, 4},
{740, 5},
{783, 6},
{833, 7},
{865, 8},
{945, 9},
{1374,10},
{1614,11},
{2202,12}
};


const QHash<int, int> sentinel2B_index_by_centralWave = {
{442, 0},
{492, 1},
{559, 2},
{665, 3},
{704, 4},
{739, 5},
{780, 6},
{833, 7},
{864, 8},
{943, 9},
{1377,10},
{1610,11},
{2186,12}
};

// clang-format on
}  // namespace sad
