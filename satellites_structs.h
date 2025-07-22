#ifndef SATELLITES_STRUCTS_H
#define SATELLITES_STRUCTS_H

#include "QString"
#include <QHash>

constexpr int LANDSAT_BANDS_NUMBER = 11;
constexpr int  SENTINEL_BANDS_NUMBER = 13;

namespace sad{

enum SATELLITE_TYPE{
    LANDSAT_9,
    LANDSAT_8,
    SENTINEL_2A,
    SENTINEL_2B,
    UNKNOWN_SATELLITE
};

extern const QString landsat9_bands_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_mult_radiance_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_add_radiance_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_mult_reflectence_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_add_reflectence_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat_bands_gui_names[LANDSAT_BANDS_NUMBER];


struct PRODUCT_CONTENTS{
    QString landsat_product_id;
    QString processing_level;
    QString file_name_bands[LANDSAT_BANDS_NUMBER];
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
    QString reflectance_mult_band[LANDSAT_BANDS_NUMBER];
    QString reflectance_add_band[LANDSAT_BANDS_NUMBER];
};

struct LANDSAT_METADATA_FILE{
    bool isHeaderValid = false;
    bool landsat9_missed_channels[LANDSAT_BANDS_NUMBER];
    PRODUCT_CONTENTS product_contents;
    IMAGE_ATTRIBUTES image_attributes;
    PROJECTION_ATTRIBUTES projection_attributes;
    LEVEL1_RADIOMETRIC_RESCALING level2_surface_reflectance_parameters;
};

// SENTINEL SECTION

extern const QString sentinel2_gui_band_names[SENTINEL_BANDS_NUMBER];

extern const QString sentinel_bands_keys[SENTINEL_BANDS_NUMBER];

extern const double sentinel_central_wave_lengths[SENTINEL_BANDS_NUMBER];

extern const QHash<const QString,QPair<int,int>> sentinel_resolutions;


struct SENTINEL_METADATA {
    bool isHeaderValid = false;
    bool sentinel_missed_channels[SENTINEL_BANDS_NUMBER];
    QString files[SENTINEL_BANDS_NUMBER];
};

struct BAND_DATA{
    double central_wave_length;
    QString gui_name;
    QString resolution_in_pixel_meters;
    double reflectance_mult;
    double reflectance_add;
    int height;
    int width;
    QString file_name;
    uint16_t* data = nullptr;
};


}


#endif // SATELLITES_STRUCTS_H
