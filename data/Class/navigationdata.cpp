#include "NavigationData.h"

QDataStream &operator>>(QDataStream &in, GNRMCData::CoordinateDefinition &coordDef) {
    int value;
    in >> value;
    coordDef = static_cast<GNRMCData::CoordinateDefinition>(value);
    return in;
}

QDataStream &operator>>(QDataStream &in, GNRMCData &data) {
    in >> data.time >> data.isValid >> data.latitude >> data.longitude >> data.speed >> data.course >> data.date;
    return in;
}

QDataStream &operator>>(QDataStream &in, GNGGAData::AltitudeUnits &altUnit) {
    int value;
    in >> value;
    altUnit = static_cast<GNGGAData::AltitudeUnits>(value);
    return in;
}

QDataStream &operator>>(QDataStream &in, ParseResult &result) {
    int value;
    in >> value;
    result = static_cast<ParseResult>(value);
    return in;
}

QDataStream& operator>>(QDataStream& in, GNGGAData& data) {
    int coordDef, altUnit, diffUnit;
    in >> data.time >> data.latitude >> data.longitude >> coordDef
        >> data.satellitesCount >> data.HDOP >> data.altitude >> altUnit
        >> data.diffElipsoidSeaLevel >> diffUnit >> data.countSecDGPS >> data.idDGPS >> data.result;
    data.coordDef = static_cast<GNGGAData::CoordinateDefinition>(coordDef);
    data.altUnit = static_cast<GNGGAData::AltitudeUnits>(altUnit);
    data.diffElipsUnit = static_cast<GNGGAData::AltitudeUnits>(diffUnit);
    return in;
}

QDataStream &operator>>(QDataStream &in, TypeGNSS &gnss) {
    int value;
    in >> value;
    gnss = static_cast<TypeGNSS>(value);
    return in;
}

QDataStream &operator>>(QDataStream &in, TypeFormat &format) {
    int value;
    in >> value;
    format = static_cast<TypeFormat>(value);
    return in;
}

QDataStream& operator>>(QDataStream& in, GNGSAData& data) {
    int format, gnss;
    in >> data.isAuto >> format;
    for (int &id : data.satellitesUsedId) in >> id;
    in >> data.PDOP >> data.HDOP >> data.VDOP >> gnss >> data.result;

    data.typeFormat = static_cast<TypeFormat>(format);
    data.typeGNSS = static_cast<TypeGNSS>(gnss);
    return in;
}

QDataStream &operator>>(QDataStream &in, GNZDAData &data) {
    in >> data.time >> data.date >> data.localOffset;
    return in;
}

QDataStream &operator>>(QDataStream &in, GNDHVData &data) {
    in >> data.time >> data.speed3D >> data.speedECEF_X >> data.speedECEF_Y >> data.speedECEF_Z >> data.speed;
    return in;
}

QDataStream &operator>>(QDataStream &in, GNGSTData &data) {
    in >> data.time >> data.rms;
    return in;
}

QDataStream &operator>>(QDataStream &in, GPTXTData &data) {
    in >> data.messageCount >> data.messageNumber >> data.messageType >> data.message;
    return in;
}

QDataStream &operator>>(QDataStream &in, GNGLLData &data) {
    in >> data.latitude >> data.longitude >> data.time >> data.isValid;
    return in;
}

QDataStream& operator>>(QDataStream& in, SatelliteInfo& data) {
    return in >> data.prn >> data.elevation >> data.azimuth >> data.snr;
}

QDataStream& operator>>(QDataStream& in, GLGSVData& data) {
    in >> data.totalMessages >> data.messageNumber >> data.satellitesCount;
    int count;
    in >> count;
    data.satelliteData.resize(count);
    for (auto& sat : data.satelliteData) in >> sat;
    in >> data.result;
    return in;
}

QDataStream &operator>>(QDataStream &in, GNVTGData &data) {
    in >> data.trueCourse >> data.magneticCourse >> data.speedKnots >> data.speedKmh >> data.isValid;
    return in;
}
