#include "google_maps_url_maker.h"
#include "cmath"
#include "iostream"

namespace{

const char httpsPart[] = R"(https://www.google.com/maps/place/)";
const char gradus[] = R"(%C2%B0)";
const char minutes[] = R"(')";
const char seconds[] = R"(%22)";
const char North[] = "N";
const char South[] = "S";
const char East[] = "E";
const char West[] = "W";
const char Plus[] = "+";
const char End[] = R"(/data=!3m2!1e3!4b1!4m4!3m3!8m2!3d46.0544!4d150.062?entry=ttu)";

}


namespace maps_utility
{

std::string convertDecimalDegridToGradusMinutesSeconds(const double &latitude,
                                                       const double &longitude){

    int latSeconds = (int)std::round(latitude * 3600);
    int latDegrees = latSeconds / 3600;
    latSeconds = std::abs(latSeconds % 3600);
    int latMinutes = latSeconds / 60;
    latSeconds %= 60;

    int longSeconds = static_cast<int>(std::round(longitude * 3600));
    int longDegrees = static_cast<int>(longSeconds / 3600);
    longSeconds = std::abs(longSeconds % 3600);
    int longMinutes = longSeconds / 60;
    longSeconds %= 60;

    std::string coordinates;
    coordinates.append(std::to_string(std::abs(latDegrees)));
    coordinates.append(gradus);
    coordinates.append(std::to_string(latMinutes));
    coordinates.append(minutes);
    coordinates.append(std::to_string(latSeconds));
    coordinates += seconds;
    if(latitude>=0){
        coordinates.append(North);
    }else{
        coordinates.append(South);
    };
    coordinates.append(Plus);
    coordinates.append(std::to_string(std::abs(longDegrees)));
    coordinates.append(gradus);
    coordinates.append(std::to_string(longMinutes));
    coordinates.append(minutes);
    coordinates.append(std::to_string(longSeconds));
    coordinates.append(seconds);
    if(longitude>=0){
        coordinates.append(East);
    }else{
        coordinates.append(West);
    }
    coordinates.append("/");
    coordinates.append("@");
    coordinates.append(std::to_string(latitude));
    coordinates.append(",");
    coordinates.append(std::to_string(longitude));
    coordinates.append(",72076m");
    coordinates.append(End);
    return coordinates;
}

std::string makeGoogleUrl(const double &latitude, const double &longitude){
    std::string url = httpsPart;
    url.append(convertDecimalDegridToGradusMinutesSeconds(latitude,longitude).c_str());
    std::cout<<url;
    return url;
}
};
