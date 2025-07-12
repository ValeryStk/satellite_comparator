#ifndef SATELLITES_STRUCTS_H
#define SATELLITES_STRUCTS_H
#include "string"
#include "QString"

#define LANDSAT_8_BANDS_NUMBER 11
#define LANDSAT_9_BANDS_NUMBER 11
#define SENTINEL_2A_BANDS_NUMBER 12

namespace sad{

enum SATELLITE_TYPE{
    LANDSAT_9,
    LANDSAT_8,
    SENTINEL_2A,
    SENTINEL_2B,
    UKNOWN_SATELLITE
};

const QString landsat9_bands_keys[LANDSAT_9_BANDS_NUMBER] = {
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
const QString landsat9_mult_radiance_keys[LANDSAT_9_BANDS_NUMBER] = {
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
const QString landsat9_add_radiance_keys[LANDSAT_9_BANDS_NUMBER] = {
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

const QString landsat9_mult_reflectence_keys[LANDSAT_9_BANDS_NUMBER] = {
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

const QString landsat9_add_reflectence_keys[LANDSAT_9_BANDS_NUMBER] = {
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


const QString landsat8_bands_gui_names[LANDSAT_8_BANDS_NUMBER] = {
    "443 nm (Aerosol) 30 m",
    "482 nm (Blue) 30 m",
    "561 nm (Green) 30 m",
    "655 nm (Red) 30 m",
    "865 nm (Near infrared) 30 m",
    "1610 nm (SWIR 1) 30 m",
    "2200 nm (SWIR 2) 30 m",
    "590 nm (Panchromatic) 15 m",
    "1375 nm (Cirrus) 30 m",
    "10900 nm (LWIR) 100 m",
    "12000 nm (LWIR) 100 m"
};

const QString landsat9_bands_gui_names[LANDSAT_9_BANDS_NUMBER] = {
    "443 nm (Aerosol) 30 m",
    "482 nm (Blue) 30 m",
    "562 nm (Green) 30 m",
    "655 nm (Red) 30 m",
    "865 nm (Near infrared) 30 m",
    "1610 nm (SWIR 1) 30 m",
    "2200 nm (SWIR 2) 30 m",
    "590 nm (Panchromatic) 15 m",
    "1375 nm (Cirrus) 30 m",
    "10800 nm (LWIR) 100 m",
    "12000 nm (LWIR) 100 m"
};


struct PRODUCT_CONTENTS{
    QString landsat_product_id;
    QString processing_level;
    QString file_name_bands[LANDSAT_9_BANDS_NUMBER];
};

struct IMAGE_ATTRIBUTES{
    QString spacecraft_id;
    QString sensor_id;
    QString date_acquired;
    QString sun_azimuth;
    QString sun_elevation;
};

struct PROJECTION_ATTRIBUTES{
    QString utm_zone;
    QString grid_cell_size_reflective;
    QString orientation;
    QString corner_ul_projection_x_product;
    QString corner_ul_projection_y_product;
};

struct LEVEL1_RADIOMETRIC_RESCALING{
    QString reflectance_mult_band[LANDSAT_9_BANDS_NUMBER];
    QString reflectance_add_band[LANDSAT_9_BANDS_NUMBER];
};

struct LANDSAT_METADATA_FILE{
    bool isHeaderValid = false;
    bool landsat9_missed_channels[LANDSAT_9_BANDS_NUMBER];
    PRODUCT_CONTENTS product_contents;
    IMAGE_ATTRIBUTES image_attributes;
    PROJECTION_ATTRIBUTES projection_attributes;
    LEVEL1_RADIOMETRIC_RESCALING level2_surface_reflectance_parameters;
};

// SENTINEL SECTION


const QString sentinel2_gui_band_names[SENTINEL_2A_BANDS_NUMBER] = {
    "443 nm (Coastal aerosol) 60 m",
    "490 nm (Blue) 10 m",
    "560 nm (Green) 10 m",
    "665 nm (Red) 10 m",
    "705 nm (Red edge 1) 20 m",
    "740 nm (Red edge 2) 20 m",
    "783 nm (Red edge 3) 20 m",
    "842 nm (NIR) 10 m",
    "865 nm (Narrow NIR) 20 m",
    "945 nm (Water vapor) 60 m",
    "1610 nm (SWIR 1) 20 m",
    "2190 nm (SWIR 2) 20 m"
};


const QString sentinel_bands_keys[SENTINEL_2A_BANDS_NUMBER] = {
    "B01",
    "B02",
    "B03",
    "B04",
    "B05",
    "B06",
    "B07",
    "B08",
    "B09",
    "B10",
    "B11",
    "B12"
};

struct SENTINEL_METADATA {
    bool isHeaderValid = false;
    bool sentinel_missed_channels[SENTINEL_2A_BANDS_NUMBER];
};


}


#endif // SATELLITES_STRUCTS_H
