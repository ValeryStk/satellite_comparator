#ifndef DBJSON_H
#define DBJSON_H
#include "QString"
#include "QVector"
#include "QJsonDocument"
#include <qdatetime.h>

class QJsonArray;
class QJsonObject;


namespace db_json {

constexpr int UNDEFINED = -999;

extern const QStringList IMAGE_TYPES_DESCRIPTION;
extern const QStringList WIND_DIRECTION_DESCRIPTION;
extern const QStringList SPECTRUM_UNITS_DESCRIPTION;
extern const QStringList CAPTURE_LEVEL_DESCRIPTION;
extern const QStringList BANDS_UNITS_DESCRIPTION;

/**
 * @brief The ImageType enum
 * Enum for image type desription
 */
enum ImageType{
    IT_UNKNOWN, IT_OBSERVE, IT_VISIR
};



/**
 * @brief The WindDirection enum
 * Enum for wind directions
 */
enum WindDirection{
    WD_UNKNOWN,
    WD_NORTH_WEST, WD_NORTH, WD_NORTH_EAST,
    WD_EAST,
    WD_SOUTH_EAST, WD_SOUTH, WD_SOUTH_WEST,
    WD_WEST,
};


/**
 * @brief The CaptureLevel enum
 * Enum for Capture Level
 */
enum CaptureLevel{
    CL_UNKNOWN, CL_LAB, CL_EARTH, CL_AVIA, CL_SPACE
};



/**
 * @brief The SpectrumUnits enum
 * Enum of graph units in spectrum
 */
enum SpectrumUnits{
    SU_UNKNOWN, SU_ADC, SU_RFL, SU_BRIGHT
};


/**
 * @brief The BandUnits enum
 * Enum of bands units in spectrum
 *
 * BU_NUMBERS     Numbers of bands
 * BU_WAVELENGTH  Waves, nm
 */
enum BandUnits{
    BU_UNKNOWN, BU_NUMBERS, BU_WAVELENGTH
};


/**
 * @brief The CLASSIFICATION struct
 * Structure describing the place in classification
 */
struct CLASSIFICATION{
    QString general_type;   //!< General type description
    QString class_name;     //!< Class name description
    QString object_name;    //!< Object name description
};

/**
 * @brief The LOCATION struct
 * Structure describing location for measurements
 */
struct LOCATION{
    double latitude = UNDEFINED;    //!< Measurements point latitude (WGS-84)
    double longitude = UNDEFINED;   //!< Measurements point longtitude (WGS-84)
    double altitude = UNDEFINED;    //!< Measurements point altitude (upper the Sea)
    QString local_name;
    QString place_name;
    QString place_type;
    QString region_name;
};

/**
 * @brief The IMAGE_OBJECT struct
 * Structure describing the image connected with the spectral data
 */
struct IMAGE_OBJECT{
    QString pathToFile;     //!< Absolute path to the image
    ImageType type;         //!< Image type
    QString description;    //!< Image text description
    QVector<QPair<int,int>> spFov;  //!< The spectrometer field of view as the vector of points
};

/**
 * @brief The FRACTION struct
 * Structure describing the fraction of materials for which spectral data was collected
 */
struct FRACTION{
    QString name;               //!< Fraction name
    double from = UNDEFINED;    //!< Fraction minimum value
    double to = UNDEFINED;      //!< Fraction maximum value
    QString unit;               //!< Fraction units
};

/**
 * @brief The AIR_CONDITIONS struct
 * Structure describing the air conditions in measurements process
 */
struct AIR_CONDITIONS{
    double temperature; //!< Temperature in the moment of measurements
    double humidity;    //!< Humidity in the moment of measurements
};

/**
 * @brief The WEATHER_CONDITIONS struct
 * Structure describing the weather conditions in measurements process
 */
struct WEATHER_CONDITIONS{
    int clouds_level = UNDEFINED;   //!< Clouds level in measurements process
    int wind = UNDEFINED;           //!< Wind level in measurements process
    WindDirection wind_direction;        //!< Wind direction
};

/**
 * @brief The META_DATA struct
 * Structure desribing meta data for the measurements
 */
struct META_DATA{
    QDateTime date_time;            //!< Maesurements date and time
    QString owner;                  //!< Maesurements data owner
    double sun_elevation_angle = UNDEFINED; //!< Sun elevation angle
    double capture_angle = UNDEFINED;       //!< Capturing angle
    QString experiment_name;        //!< Name of the experiment
    CaptureLevel capture_level;     //!< The level of measurements
    CLASSIFICATION classification;  //!< Spectra place in classification
    LOCATION location;              //!< Measurements location description
    QVector<IMAGE_OBJECT> images;   //!< Images connected to spectra description
    QString relief_type;            //!< The type of relief in the measurements area
    FRACTION fraction;              //!< Measured materials fraction
    AIR_CONDITIONS air_conditions;  //!< Measurements air conditions
    WEATHER_CONDITIONS weather_conditions;  //!< Measurements weather conditions
};

/**
 * @brief The SPECTRAL_ATTRIBUTES struct
 * Structure needed to describe spectral attributes
 */
struct SPECTRAL_ATTRIBUTES{
    QString instrument;     //!< Measurements instrument description
    SpectrumUnits type;     //!< Spectrum units
    QString description;    //!< Spectrum description
};

/**
 * @brief The SPECTRAL_DATA struct
 * Structure needed to desribe spectral data
 */
struct SPECTRAL_DATA{
    QVector<SPECTRAL_ATTRIBUTES> attributes;    //!< Spectral attributes
    QVector<QVector<double>> waves;             //!< Spectral data waves vector
    QVector<QVector<double>> values;            //!< Spectral data values vector
};

/**
 * @brief The SPECTRAL_STRUCT struct
 * Structure needed to desribe spectra with its metadata
 */
struct SPECTRAL_STRUCT {
    META_DATA md;       //!< Measurements metadata
    SPECTRAL_DATA sd;   //!< Measurements spectrum data
};

bool getJsonObjectFromFile(const QString &path,
                           QJsonObject &object);

CaptureLevel getCaptureLevelFromStr(QString captureLevel);

ImageType getImageTypeFromStr(QString imageType);

WindDirection getWindDirectionFromStr(QString windDir);

SpectrumUnits getSpectrumUnitsFromStr(QString spectrumUnits);

SPECTRAL_STRUCT getStructFromJsonObject(const QJsonObject &json_object);

bool saveJsonObjectToFile(const QString &path,
                          const QJsonObject &json_object,
                          QJsonDocument::JsonFormat format);

bool saveStructToJsonFile(const QString &path,
                          const SPECTRAL_STRUCT &spectral_struct,
                          QJsonDocument::JsonFormat format);

void getJsonObjectFromStruct(const SPECTRAL_STRUCT &spectral_struct,
                             QJsonObject &json_object);

bool copyStructLikeJsonToClipboard(const SPECTRAL_STRUCT &spectral_struct);

bool isAllNodesExist(const QJsonObject &object);

void makeStructCleared(SPECTRAL_STRUCT &spectral_struct);

QString getStringMdFromStruct(META_DATA metaData);

QString getStringSpecAttrsFromStruct(SPECTRAL_ATTRIBUTES specAttrs);

} // end namespace db_json

#endif // DBJSON_H
