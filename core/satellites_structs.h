#ifndef SATELLITES_STRUCTS_H
#define SATELLITES_STRUCTS_H

#include "QString"
#include <QHash>
#include <QDateTime>
#include <QVector>

constexpr int LANDSAT_BANDS_NUMBER = 11;
constexpr int  SENTINEL_BANDS_NUMBER = 13;

namespace sad{

enum SATELLITE_TYPE{
    LANDSAT_9,
    LANDSAT_8,
    SENTINEL_2A,
    SENTINEL_2B,
    TIME_ROW_COMBINATION,
    UNKNOWN_SATELLITE
};

extern const QString landsat9_bands_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_mult_radiance_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_add_radiance_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_mult_reflectence_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat9_add_reflectence_keys[LANDSAT_BANDS_NUMBER];

extern const QString landsat_bands_gui_names[LANDSAT_BANDS_NUMBER];

extern const double landsat_central_wavelengths[LANDSAT_BANDS_NUMBER];

extern const int sorted_landsat_bands_order_by_wavelength[LANDSAT_BANDS_NUMBER];


struct geoTransform {
    double ulX = 0;           // Верхний левый X (восточное направление)
    double resX = 0;          // Разрешение по X
    double rotateX = 0;       // Поворот X (обычно 0 для Landsat)
    double ulY = 0;           // Верхний левый Y (северное направление)
    double rotateY = 0;       // Поворот Y (обычно 0 для Landsat)
    double resY = 0;          // Разрешение по Y (отрицательное, т.к. ось Y направлена вниз)
    double utmZone = 0;
};

struct PRODUCT_CONTENTS{
    QString landsat_product_id;
    QDateTime date_time;
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
extern const QString sentinel_2A_gui_band_names[SENTINEL_BANDS_NUMBER];
extern const double sentinel_2A_central_wave_lengths[SENTINEL_BANDS_NUMBER];
extern const QString sentinel_2B_gui_band_names[SENTINEL_BANDS_NUMBER];
extern const double sentinel_2B_central_wave_lengths[SENTINEL_BANDS_NUMBER];

extern const QHash<const QString,QPair<int,int>> sentinel_resolutions;
extern const QString sentinel_bands_keys[SENTINEL_BANDS_NUMBER];


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

struct QA_MASK_DATA {
    int height;
    int width;
    QString file_name;
    uint16_t* data = nullptr;
};

struct DATA_CLOUD_SHADOW_MASK{
bool is_data;
bool is_cloud;
bool is_shadow;
bool is_common_valid;
};


struct BANDS_FOR_CALCULATING_INDEXES{
    double RED_BAND;
    double NIR_BAND;
    double SWIR1_BAND;
};

struct NDWI_NDVI_TIME_ROW{
   QVector<double> ndvi_time_row;
   QVector<double> ndwi_time_row;
   QVector<double> slopes;
};

}


#endif // SATELLITES_STRUCTS_H
