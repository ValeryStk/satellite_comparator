#ifndef SATELLITES_STRUCTS_H
#define SATELLITES_STRUCTS_H
#include "string"
#include "QString"

#define LANDSAT_9_BANDS_NUMBER 11

namespace sad{

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
    std::string landsat_product_id;
    std::string processing_level;
    std::string file_name_bands[LANDSAT_9_BANDS_NUMBER];
};

struct IMAGE_ATTRIBUTES{
    std::string spacecraft_id;
    std::string sensor_id;
    std::string date_acquired;
    std::string sun_azimuth;
    std::string sun_elevation;
};

struct PROJECTION_ATTRIBUTES{
    std::string utm_zone;
    std::string grid_cell_size_reflective;
    std::string orientation;
    std::string corner_ul_projection_x_product;
    std::string corner_ul_projection_y_product;
};

struct LEVEL1_RADIOMETRIC_RESCALING{
    std::string reflectance_mult_band[LANDSAT_9_BANDS_NUMBER];
    std::string reflectance_add_band[LANDSAT_9_BANDS_NUMBER];
};

struct LANDSAT_METADATA_FILE{
    bool isHeaderValid = false;
    PRODUCT_CONTENTS product_contents;
    IMAGE_ATTRIBUTES image_attributes;
    PROJECTION_ATTRIBUTES projection_attributes;
    LEVEL1_RADIOMETRIC_RESCALING level1_radiometric_rescaling;
};

}


#endif // SATELLITES_STRUCTS_H
