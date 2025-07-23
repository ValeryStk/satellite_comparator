#include "satellites_structs.h"

namespace sad {


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
    "B01 - 443 nm (Aerosol) 30 m",
    "B02 - 482 nm (Blue) 30 m",
    "B03 - 562 nm (Green) 30 m",
    "B04 - 655 nm (Red) 30 m",
    "B05 - 865 nm (Near infrared) 30 m",
    "B06 - 1610 nm (SWIR 1) 30 m",
    "B07 - 2200 nm (SWIR 2) 30 m",
    "B08 - 590 nm (Panchromatic) 15 m",
    "B09 - 1375 nm (Cirrus) 30 m",
    "B10 - 10800 nm (LWIR) 100 m",
    "B11 - 12000 nm (LWIR) 100 m"
};





// SENTINEL-2A
const QString sentinel_2A_gui_band_names[SENTINEL_BANDS_NUMBER] = {
    "B01 - 443 nm aerosol",
    "B02 - 493 nm blue",
    "B03 - 560 nm green",
    "B04 - 665 nm red",
    "B05 - 704 nm VNIR",
    "B06 - 740 nm VNIR",
    "B07 - 783 nm VNIR",
    "B08 - 833 nm VNIR",
    "B8A - 865 nm SWIR",
    "B09 - 945 nm SWIR",
    "B10 - 1374 nm SWIR",
    "B11 - 1614 nm SWIR",
    "B12 - 2202 nm SWIR"
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
    "B01 - 442 nm aerosol",
    "B02 - 492 nm blue",
    "B03 - 559 nm green",
    "B04 - 665 nm red",
    "B05 - 704 nm VNIR",
    "B06 - 739 nm VNIR",
    "B07 - 780 nm VNIR",
    "B08 - 833 nm VNIR",
    "B8A - 864 nm SWIR",
    "B09 - 943 nm SWIR",
    "B10 - 1377 nm SWIR",
    "B11 - 1610 nm SWIR",
    "B12 - 2186 nm SWIR"
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
    "B09"
    "B10",
    "B11",
    "B12"
};


const QHash<const QString,QPair<int,int>> sentinel_resolutions = {
    {"R10m",{10980,10980}},
    {"R20m",{5490,5490}},
    {"R60m",{1830,1830}}
};


}
