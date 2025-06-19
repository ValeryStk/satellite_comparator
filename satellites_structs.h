#ifndef SATELLITES_STRUCTS_H
#define SATELLITES_STRUCTS_H
#include "string"

namespace sas{

struct PRODUCT_CONTENTS{
    std::string landsat_product_id;
    std::string processing_level;
    std::string file_name_bands[11];
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
    std::string reflectance_mult_band[9];
    std::string reflectance_add_band[9];
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
