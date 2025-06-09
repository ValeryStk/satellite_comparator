#include "DBJson.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDebug>
#include <QClipboard>
#include <QGuiApplication>
#include <QDateTime>


namespace {

bool checkNodes(const QJsonObject &obj,
                const QStringList &nodes){
    for(int i=0;i < nodes.size();++i){
        if(obj.find(nodes[i]) == obj.end())
            return false;
    }
    return true;
};

} // end namespace

namespace db_json {

bool getJsonObjectFromFile(const QString &path,
                           QJsonObject &object)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "File can't be opened!" <<path;
        return false;
    };
    QByteArray data = file.readAll();
    QJsonParseError errorPtr;
    object = QJsonDocument::fromJson(data, &errorPtr).object();
    if (object.isEmpty()) {
        qDebug() << "JSON IS EMPTY: "<<errorPtr.errorString();
        return false;
    }
    if(!isAllNodesExist(object)){
        qDebug() << "NOT ALL NODES EXISTS IN JSON FILE...";
        return false;
    }
    file.close();
    return true;
}

SPECTRAL_STRUCT getStructFromJsonObject(const QJsonObject &json_object)
{
    SPECTRAL_STRUCT spectral_struct;
    auto md = json_object.find("meta_data").value().toObject();
    spectral_struct.md.date_time = QDateTime::fromString(md.find("date_time").value().toString(), "yyyy_MM_dd_hh_mm_ss_zzz");
    spectral_struct.md.owner = md.find("owner").value().toString();
    spectral_struct.md.sun_elevation_angle = md.find("sun_elevation_angle").value().toDouble();
    spectral_struct.md.capture_angle = md.find("capture_angle").value().toDouble();
    spectral_struct.md.experiment_name = md.find("experiment_name").value().toString();
    spectral_struct.md.capture_level = getCaptureLevelFromStr(md.find("capture_type").value().toString());
    auto classification = md.find("classification")->toObject();
    spectral_struct.md.classification.general_type = classification.find("general_type")->toString();
    spectral_struct.md.classification.class_name = classification.find("class_name")->toString();
    spectral_struct.md.classification.object_name = classification.find("object_name")->toString();
    auto location = md.find("location")->toObject();
    spectral_struct.md.location.latitude = location.find("latitude").value().toDouble();
    spectral_struct.md.location.longitude = location.find("longitude").value().toDouble();
    spectral_struct.md.location.altitude = location.find("altitude").value().toDouble();
    spectral_struct.md.location.local_name = location.find("local_name").value().toString();
    spectral_struct.md.location.place_name = location.find("place_name").value().toString();
    spectral_struct.md.location.place_type = location.find("place_type").value().toString();
    spectral_struct.md.location.region_name = location.find("region_name").value().toString();
    auto images = md.find("images")->toArray();
    for(auto &&it:images){
        auto image_object = it.toObject();
        IMAGE_OBJECT img_obj;
        img_obj.pathToFile = image_object.find("name")->toString();
        img_obj.type = getImageTypeFromStr(image_object.find("type")->toString());
        img_obj.description = image_object.find("description")->toString();
        QVector<QPair<int,int>> fov;
        auto fov_x = image_object.find("spectrometer_fov_x")->toArray();
        auto fov_y = image_object.find("spectrometer_fov_y")->toArray();
        for(int i = 0; i < fov_x.count(); i++){
            QPair<int,int> point;
            point.first = fov_x.at(i).toInt();
            point.second = fov_y.at(i).toInt();
            fov.append(point);
        }
        img_obj.spFov = fov;
        spectral_struct.md.images.append(img_obj);
    };
    spectral_struct.md.relief_type = md.find("relief_type").value().toString();
    auto fraction = md.find("fraction").value().toObject();
    spectral_struct.md.fraction.name = fraction.find("name")->toString();
    spectral_struct.md.fraction.from = fraction.find("from")->toDouble();
    spectral_struct.md.fraction.to = fraction.find("to")->toDouble();
    spectral_struct.md.fraction.unit = fraction.find("unit")->toString();
    auto air_conditions = md.find("air_conditions")->toObject();
    spectral_struct.md.air_conditions.temperature = air_conditions.find("temperature").value().toDouble();
    spectral_struct.md.air_conditions.humidity = air_conditions.find("humidity").value().toDouble();
    auto weather_conditions = md.find("weather_conditions").value().toObject();
    spectral_struct.md.weather_conditions.clouds_level = weather_conditions.find("clouds_level")->toDouble();
    spectral_struct.md.weather_conditions.wind = weather_conditions.find("wind")->toDouble();
    spectral_struct.md.weather_conditions.wind_direction = getWindDirectionFromStr(weather_conditions.find("direction")->toString());
    auto sd = json_object.find("spectral_data").value().toObject();
    auto attributes = sd.find("attributes")->toArray();
    for(int i=0; i<attributes.size(); ++i){
        db_json::SPECTRAL_ATTRIBUTES sa;
        auto atr = attributes[i].toObject();
        sa.instrument = atr.find("instrument")->toString();
        sa.type = getSpectrumUnitsFromStr(atr.find("type")->toString());
        sa.description = atr.find("description")->toString();
        spectral_struct.sd.attributes.append(sa);
    }
    auto waves = sd.find("waves").value().toArray();
    for(auto &&it:waves){
        auto wave_line = it.toArray();
        QVector<double> wave;
        for(auto &&it:wave_line){
            wave.push_back(it.toDouble());
        }
        spectral_struct.sd.waves.append(wave);
    };
    auto values = sd.find("values").value().toArray();
    for(auto &&it:values){
        auto value_line = it.toArray();
        QVector<double> value;
        for(auto &&it:value_line){
            value.push_back(it.toDouble());
        }
        spectral_struct.sd.values.append(value);
    };

    return spectral_struct;
}

bool saveJsonObjectToFile(const QString &path,
                          const QJsonObject &json_object,
                          QJsonDocument::JsonFormat format)
{
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))return false;
    auto json_doc = QJsonDocument(json_object).toJson(format);
    auto result = file.write(json_doc);
    file.close();
    if(result == -1)return false;
    else return true;
}

bool saveStructToJsonFile(const QString &path,
                          const SPECTRAL_STRUCT &spectral_struct,
                          QJsonDocument::JsonFormat format)
{
    QJsonObject root;
    getJsonObjectFromStruct(spectral_struct,root);
    return saveJsonObjectToFile(path,root,format);
}

bool isAllNodesExist(const QJsonObject &object)
{
    QStringList nodes = {
        "meta_data",
        "spectral_data"
    };
    if(!checkNodes(object,nodes))return false;
    nodes.clear();
    nodes << "date_time" << "owner" << "sun_elevation_angle"
          << "capture_angle" << "experiment_name" << "capture_type"
          << "classification" << "location"<<"images"
          << "relief_type" << "fraction" << "air_conditions"
          << "weather_conditions";
    QJsonObject md = object.find("meta_data")->toObject();
    if(!checkNodes(md,nodes))return false;
    return true;
}

bool copyStructLikeJsonToClipboard(const SPECTRAL_STRUCT &spectral_struct)
{
    QJsonObject jo;
    getJsonObjectFromStruct(spectral_struct,jo);
    QJsonDocument doc(jo);
    QString strJson(doc.toJson(QJsonDocument::Indented));
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(strJson);
    return true;
}

void getJsonObjectFromStruct(const SPECTRAL_STRUCT &spectral_struct, QJsonObject &root)
{
    QJsonObject meta_data_object;
    meta_data_object["date_time"] = spectral_struct.md.date_time.toString("yyyy_MM_dd_hh_mm_ss_zzz");
    meta_data_object["owner"] = spectral_struct.md.owner;
    meta_data_object["sun_elevation_angle"] = spectral_struct.md.sun_elevation_angle;
    meta_data_object["capture_angle"] = spectral_struct.md.capture_angle;
    meta_data_object["experiment_name"] = spectral_struct.md.experiment_name;
    meta_data_object["capture_type"] = spectral_struct.md.capture_level;

    QJsonObject classification;
    classification["general_type"] = spectral_struct.md.classification.general_type;
    classification["class_name"] = spectral_struct.md.classification.class_name;
    classification["object_name"] = spectral_struct.md.classification.object_name;
    meta_data_object["classification"] = classification;

    QJsonObject location;
    location["latitude"] = spectral_struct.md.location.latitude;
    location["longitude"] = spectral_struct.md.location.longitude;
    location["altitude"] = spectral_struct.md.location.altitude;
    location["local_name"] = spectral_struct.md.location.local_name;
    location["place_name"] = spectral_struct.md.location.place_name;
    location["place_type"] = spectral_struct.md.location.place_type;
    location["region_name"] = spectral_struct.md.location.region_name;
    meta_data_object["location"] = location;

    QJsonArray images_array;
    for(int i = 0;i<spectral_struct.md.images.size();++i){
        QJsonArray fov_array_x, fov_array_y;
        foreach(auto point, spectral_struct.md.images[i].spFov){
            fov_array_x.append(point.first);
            fov_array_y.append(point.second);
        }
        images_array.append(QJsonObject(
                                {{"name",spectral_struct.md.images[i].pathToFile},
                                 {"type",IMAGE_TYPES_DESCRIPTION.at(spectral_struct.md.images[i].type)},
                                 {"description",spectral_struct.md.images[i].description},
                                 {"spectrometer_fov_x", fov_array_x},
                                 {"spectrometer_fov_y", fov_array_y}
                                }));
    }
    meta_data_object["images"] = images_array;
    meta_data_object["relief_type"] = spectral_struct.md.relief_type;

    QJsonObject fraction;
    fraction["name"] = spectral_struct.md.fraction.name;
    fraction["from"] = spectral_struct.md.fraction.from;
    fraction["to"] = spectral_struct.md.fraction.to;
    fraction["unit"] = spectral_struct.md.fraction.unit;
    meta_data_object["fraction"] = fraction;

    QJsonObject air_conditions;
    air_conditions["temperature"] = spectral_struct.md.air_conditions.temperature;
    air_conditions["humidity"] = spectral_struct.md.air_conditions.humidity;
    meta_data_object["air_conditions"] = air_conditions;

    QJsonObject weather_conditions;
    weather_conditions["clouds_level"] = spectral_struct.md.weather_conditions.clouds_level;
    weather_conditions["wind"] = spectral_struct.md.weather_conditions.wind;
    weather_conditions["direction"] = WIND_DIRECTION_DESCRIPTION.at(spectral_struct.md.weather_conditions.wind_direction);
    meta_data_object["weather_conditions"] = weather_conditions;

    QJsonObject spectral_data;

    QJsonArray attributes;
    for(int i=0; i<spectral_struct.sd.attributes.size(); ++i){
        attributes.append(QJsonValue({
                                        {"instrument",spectral_struct.sd.attributes[i].instrument},
                                        {"type", SPECTRUM_UNITS_DESCRIPTION.at(spectral_struct.sd.attributes[i].type)},
                                        {"description",spectral_struct.sd.attributes[i].description}
                                    }));
    }
    spectral_data["attributes"] = attributes;
    QJsonArray waves;
    for(int i=0;i<spectral_struct.sd.waves.size();++i){
        QJsonArray waves_line;
        for(int j=0;j<spectral_struct.sd.waves[i].size();++j){
            waves_line.append(spectral_struct.sd.waves[i][j]);
        }
        waves.append(waves_line);
    }
    spectral_data["waves"] = waves;

    QJsonArray values;
    for(int i=0;i<spectral_struct.sd.values.size();++i){
        QJsonArray values_line;
        for(int j=0;j<spectral_struct.sd.values[i].size();++j){
            values_line.append(spectral_struct.sd.values[i][j]);
        }
        values.append(values_line);
    }
    spectral_data["values"] = values;

    root["meta_data"] = meta_data_object;
    root["spectral_data"] = spectral_data;

}

void makeStructCleared(SPECTRAL_STRUCT &spectral_struct)
{
    spectral_struct.md.date_time = QDateTime::currentDateTime();
    spectral_struct.md.owner = "";
    spectral_struct.md.experiment_name = "";
    spectral_struct.md.capture_level = CL_UNKNOWN;
    spectral_struct.md.sun_elevation_angle = UNDEFINED;
    spectral_struct.md.capture_angle = UNDEFINED;

    spectral_struct.md.location.local_name = "";
    spectral_struct.md.location.place_name = "";
    spectral_struct.md.location.region_name = "";
    spectral_struct.md.location.place_type = "";
    spectral_struct.md.location.latitude = UNDEFINED;
    spectral_struct.md.location.longitude = UNDEFINED;
    spectral_struct.md.location.altitude = UNDEFINED;

    spectral_struct.md.classification.class_name = "";
    spectral_struct.md.classification.general_type = "";
    spectral_struct.md.classification.object_name = "";

    spectral_struct.md.air_conditions.humidity = UNDEFINED;
    spectral_struct.md.air_conditions.temperature = UNDEFINED;

    spectral_struct.md.weather_conditions.clouds_level = UNDEFINED;
    spectral_struct.md.weather_conditions.wind = UNDEFINED;
    spectral_struct.md.weather_conditions.wind_direction = WD_UNKNOWN;

    spectral_struct.md.fraction.from = UNDEFINED;
    spectral_struct.md.fraction.to = UNDEFINED;
    spectral_struct.md.fraction.name = "";
    spectral_struct.md.fraction.unit = "";

    spectral_struct.md.images.clear();
    spectral_struct.sd.attributes.clear();
    spectral_struct.sd.waves.clear();
    spectral_struct.sd.values.clear();

}

CaptureLevel getCaptureLevelFromStr(QString captureLevel)
{
    CaptureLevel resLevel = CL_UNKNOWN;
    if(captureLevel.contains("Лабораторный", Qt::CaseInsensitive))
        resLevel = CL_LAB;
    else if(captureLevel.contains("Наземный", Qt::CaseInsensitive))
        resLevel = CL_EARTH;
    else if (captureLevel.contains("Авиационный", Qt::CaseInsensitive))
        resLevel = CL_AVIA;
    else if (captureLevel.contains("Космический", Qt::CaseInsensitive))
        resLevel = CL_SPACE;
    return resLevel;
}

ImageType getImageTypeFromStr(QString imageType)
{
    ImageType resType = IT_UNKNOWN;
    if(imageType.contains("Обзорное"))
        resType = IT_OBSERVE;
    else if(imageType.contains("Визирное"))
        resType = IT_VISIR;
    else
        resType = IT_UNKNOWN;
    return resType;
}

SpectrumUnits getSpectrumUnitsFromStr(QString spectrumUnits)
{
    SpectrumUnits resUnits = SU_UNKNOWN;
    if(spectrumUnits.contains("АЦП", Qt::CaseInsensitive))
        resUnits = SU_ADC;
    else if(spectrumUnits.contains("КСЯ", Qt::CaseInsensitive))
        resUnits = SU_RFL;
    else if(spectrumUnits.contains("СПЭЯ", Qt::CaseInsensitive))
        resUnits = SU_BRIGHT;
    return resUnits;
}

WindDirection getWindDirectionFromStr(QString windDir)
{
    WindDirection resDir = WD_UNKNOWN;
    if(windDir.contains("север", Qt::CaseInsensitive)){
        if(windDir.contains("запад", Qt::CaseInsensitive))
            resDir = WD_NORTH_WEST;
        else if (windDir.contains("вост", Qt::CaseInsensitive))
            resDir = WD_NORTH_EAST;
        else
            resDir = WD_NORTH;
    }else if(windDir.contains("ю", Qt::CaseInsensitive)){
        if(windDir.contains("запад", Qt::CaseInsensitive))
            resDir = WD_SOUTH_WEST;
        else if (windDir.contains("вост", Qt::CaseInsensitive))
            resDir = WD_SOUTH_EAST;
        else
            resDir = WD_SOUTH;
    }else if(windDir.contains("запад", Qt::CaseInsensitive)){
        resDir = WD_WEST;
    }else if(windDir.contains("вост", Qt::CaseInsensitive)){
        resDir = WD_EAST;
    }
    return resDir;
}

QString getStringMdFromStruct(META_DATA metaData)
{
    QString mdString;
    mdString.append("Владелец:\t" + metaData.owner + "\n");
    mdString.append("Эксперимент:\t" + metaData.experiment_name + "\n");
    mdString.append("Уровень съемки:\t" + CAPTURE_LEVEL_DESCRIPTION.at(metaData.capture_level) + "\n");
    mdString.append("Дата:\t" + metaData.date_time.date().toString("dd.MM.yyyy") + "\n");
    mdString.append("Время:\t" + metaData.date_time.time().toString("hh:mm:ss.zzz") + "\n");
    mdString.append("Местоположение:\t" + metaData.location.place_name + "\n");
    if(metaData.location.latitude > -90 && metaData.location.latitude < 90)
        mdString.append("Широта:\t" + QString::number(metaData.location.latitude, 'f', 2) + "\n");
    if(metaData.location.longitude > -180 && metaData.location.longitude < 180)
        mdString.append("Долгота:\t" + QString::number(metaData.location.longitude, 'f', 2) + "\n");
    if(metaData.location.altitude > 0)
        mdString.append("Высота, м н.у.м.:\t" + QString::number(metaData.location.altitude, 'f', 2) + "\n");
    mdString.append("Угол спектрометрирования, град.:\t" + QString::number(metaData.capture_angle, 'f', 2) + "\n");
    mdString.append("Угол Солнца, град.:\t" + QString::number(metaData.sun_elevation_angle, 'f', 2) + "\n");
    mdString.append("Температура, °С:\t" + QString::number(metaData.air_conditions.temperature, 'f', 2) + "\n");
    mdString.append("Влажность, %:\t" + QString::number(metaData.air_conditions.humidity, 'f', 2) + "\n");
    mdString.append("Уровень облачности:\t" + QString::number(metaData.weather_conditions.clouds_level) + "\n");
    mdString.append("Ветер, м/с:\t" + QString::number(metaData.weather_conditions.wind));
    if(metaData.weather_conditions.wind_direction != WD_UNKNOWN)
        mdString.append(", "+ WIND_DIRECTION_DESCRIPTION.at(metaData.weather_conditions.wind_direction) + "\n\n");
    else
        mdString.append("\n\n");

    return mdString;
}

QString getStringSpecAttrsFromStruct(SPECTRAL_ATTRIBUTES specAttrs)
{
    QString saString;
    saString.append("Прибор:\t" + specAttrs.instrument + "\n");
    qDebug()<<specAttrs.type<<SPECTRUM_UNITS_DESCRIPTION.at(specAttrs.type);
    saString.append("Единицы измерения по оси Y:\t" + SPECTRUM_UNITS_DESCRIPTION.at(specAttrs.type) + "\n");
    saString.append("Единицы измерения по оси X:\tДлина волны, нм\n");
    saString.append("Описание:\t" + specAttrs.description + "\n\n");
    return saString;
}

} // end db_json namespace
