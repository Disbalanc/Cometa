#include "formatnavigationdata.h"

NavigationDataFormatter::NavigationDataFormatter(QObject *parent)
    : QObject(parent)
{
    // Инициализация кэша форматирования
    m_typeNames = {
        {MsgType::GNRMC, "GNRMC"}, {MsgType::GNGGA, "GNGGA"},
        {MsgType::GNGSA, "GNGSA"},     {MsgType::GNZDA, "GNZDA"},
        {MsgType::GNDHV, "GNDHV"}, {MsgType::GNGST, "GNGST"},
        {MsgType::GPTXT, "GPTXT"}, {MsgType::GNGLL, "GNGLL"},
        {MsgType::GLGSV, "GLGSV"}, {MsgType::GNVTG, "GNVTG"}
    };
}

QString NavigationDataFormatter::formatNavigationData(const NavigationData &navData) {
    QString formattedData;
    formattedData.reserve(1024); // Резервируем память заранее

    // Используем QStringBuilder для эффективной конкатенации
    formattedData += u"Parsing Timestamp: "
                     % navData.timestamp.toString(u"yyyy-MM-dd hh:mm:ss")
                     % u"\n\n";

    formattedData += formatNavDataByType(navData);
    return formattedData;
}

QString NavigationDataFormatter::formatNavDataByType(const NavigationData &navData) {
    // Используем switch с возвратом строки для оптимизации
    switch (navData.type) {
    case MsgType::GNRMC: {
        GNRMCData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNRMCData(data);
    }
    case MsgType::GNGGA: {
        GNGGAData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNGGAData(data);
    }
    case MsgType::GNGSA: {
        GNGSAData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGSAData(data);
    }
    case MsgType::GNZDA: {
        GNZDAData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNZDAData(data);
    }
    case MsgType::GNDHV: {
        GNDHVData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNDHVData(data);
    }
    case MsgType::GNGST: {
        GNGSTData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNGSTData(data);
    }
    case MsgType::GPTXT: {
        GPTXTData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGPTXTData(data);
    }
    case MsgType::GNGLL: {
        GNGLLData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNGLLData(data);
    }
    case MsgType::GLGSV: {
        GLGSVData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGLGSVData(data);
    }
    case MsgType::GNVTG: {
        GNVTGData data;
        QDataStream stream(navData.data);
        stream >> data; // Используем оператор десериализации
        return formatGNVTGData(data);
    }
    default:
        return QString("Unknown Message Type: %1\n").arg(static_cast<int>(navData.type));
    }
}

QString NavigationDataFormatter::formatGNRMCData(const GNRMCData &data) {
    return QStringLiteral(
               "  Time: %1\n"
               "  Date: %2\n"
               "  Latitude: %3\n"
               "  Longitude: %4\n"
               "  Speed: %5 knots\n"
               "  Course: %6°\n"
               "  Valid: %7\n")
        .arg(data.time.toString("HH:mm:ss.zzz"),
             data.date.toString("dd.MM.yyyy"),
             QString::number(data.latitude, 'f', 6),
             QString::number(data.longitude, 'f', 6),
             QString::number(data.speed, 'f', 1),
             QString::number(data.course, 'f', 1),
             data.isValid ? "Yes" : "No");
}

QString NavigationDataFormatter::formatGNGGAData(const GNGGAData &data) {
    QString base = QStringLiteral(
                       "  Time: %1\n"
                       "  Latitude: %2\n"
                       "  Longitude: %3\n"
                       "  Quality: %4\n"
                       "  Satellites: %5\n"
                       "  HDOP: %6\n"
                       "  Altitude: %7 %8\n"
                       "  Geoid Separation: %9 %10\n"
                       "  DGPS Age: %11\n"
                       "  DGPS ID: %12\n")
                       .arg(data.time.toString("HH:mm:ss.zzz"),
                            QString::number(data.latitude, 'f', 6),
                            QString::number(data.longitude, 'f', 6),
                            QString::number(static_cast<int>(data.coordDef)),
                            QString::number(data.satellitesCount),
                            QString::number(static_cast<double>(data.HDOP) / 10.0, 'f', 1));

    return base.arg(QString::number(data.altitude, 'f', 2),
                    data.altUnit == GNGGAData::METER ? "m" : "ft",
                    QString::number(data.diffElipsoidSeaLevel, 'f', 2),
                    data.diffElipsUnit == GNGGAData::METER ? "m" : "ft",
                    data.countSecDGPS > 0 ? QString::number(static_cast<double>(data.countSecDGPS) / 10.0, 'f', 1) : "N/A",
                    data.idDGPS > 0 ? QString::number(data.idDGPS) : "N/A");
}

QString NavigationDataFormatter::formatGSAData(const GNGSAData &data) {
    QStringList satellites;
    for (int id : data.satellitesUsedId) {
        if (id > 0) {
            satellites << QString::number(id);
        }
    }

    return QStringLiteral(
               "  Auto: %1\n"
               "  Fix Type: %2D\n"
               "  Satellites Used: %3\n"
               "  PDOP: %4\n"
               "  HDOP: %5\n"
               "  VDOP: %6\n"
               "  GNSS: %7\n")
        .arg(data.isAuto ? "Yes" : "No",
             QString::number(static_cast<int>(data.typeFormat) + 1),
             satellites.join(", "),
             QString::number(static_cast<double>(data.PDOP) / 10.0, 'f', 1),
             QString::number(static_cast<double>(data.HDOP) / 10.0, 'f', 1),
             QString::number(static_cast<double>(data.VDOP) / 10.0, 'f', 1),
             gnssTypeToString(data.typeGNSS));
}

QString NavigationDataFormatter::formatGNZDAData(const GNZDAData &data) {
    const int hours = data.localOffset / 60;
    const int minutes = data.localOffset % 60;

    return QStringLiteral(
               "  UTC Time: %1\n"
               "  Date: %2\n"
               "  Local Offset: UTC%3%4:%5\n")
        .arg(data.time.toString("HH:mm:ss.zzz"),
             data.date.toString("dd.MM.yyyy"),
             hours >= 0 ? "+" : "-",
             QString::number(qAbs(hours)).rightJustified(2, '0'),
             QString::number(minutes).rightJustified(2, '0'));
}

QString NavigationDataFormatter::formatGNDHVData(const GNDHVData &data) {
    return QStringLiteral(
               "  Time: %1\n"
               "  3D Speed: %2 m/s\n"
               "  ECEF X: %3 m/s\n"
               "  ECEF Y: %4 m/s\n"
               "  ECEF Z: %5 m/s\n"
               "  Ground Speed: %6 m/s\n")
        .arg(data.time.toString("HH:mm:ss.zzz"),
             QString::number(data.speed3D, 'f', 2),
             QString::number(data.speedECEF_X, 'f', 2),
             QString::number(data.speedECEF_Y, 'f', 2),
             QString::number(data.speedECEF_Z, 'f', 2),
             QString::number(data.speed, 'f', 2));
}

QString NavigationDataFormatter::formatGNGSTData(const GNGSTData &data) {
    return QStringLiteral(
               "  Time: %1\n"
               "  RMS: %2\n")
        .arg(data.time.toString("HH:mm:ss.zzz"),
             QString::number(data.rms, 'f', 2));
}

QString NavigationDataFormatter::formatGPTXTData(const GPTXTData &data) {
    return QStringLiteral(
               "  Message: %1\n"
               "  Type: %2\n"
               "  Number: %3\n")
        .arg(data.message,
             QString::number(data.messageType),
             QString::number(data.messageNumber));
}

QString NavigationDataFormatter::formatGNGLLData(const GNGLLData &data) {
    return QStringLiteral(
               "  Latitude: %1\n"
               "  Longitude: %2\n"
               "  Time: %3\n"
               "  Valid: %4\n")
        .arg(formatCoordinate(data.latitude, true),
             formatCoordinate(data.longitude, false),
             data.time.toString("HH:mm:ss.zzz"),
             data.isValid ? "Yes" : "No");
}

QString NavigationDataFormatter::formatGNVTGData(const GNVTGData &data) {
    return QStringLiteral(
               "  Course: %1°\n"
               "  Ground Speed: %2 knots\n"
               "  Ground Speed (km/h): %3 km/h\n")
        .arg(QString::number(data.trueCourse, 'f', 1),
             QString::number(data.speedKnots, 'f', 1),
             QString::number(data.speedKmh, 'f', 1));
}

QString NavigationDataFormatter::formatGLGSVData(const GLGSVData &data) {
    QStringList satellites;
    for (const SatelliteInfo &sat : data.satelliteData) {
        satellites.append(QString("  PRN: %1  Elev: %2°  Azim: %3°  SNR: %4 dBHz")
                              .arg(sat.prn)
                              .arg(sat.elevation, 0, 'f', 1)
                              .arg(sat.azimuth, 0, 'f', 1)
                              .arg(sat.snr, 0, 'f', 1));
    }

    return QStringLiteral(
               "  Total Messages: %1\n"
               "  Current Message: %2\n"
               "  Satellites in View: %3\n"
               "  Satellites Data:\n%4\n")
        .arg(data.totalMessages)
        .arg(data.messageNumber)
        .arg(data.satellitesCount)
        .arg(satellites.join("\n"));
}

QString NavigationDataFormatter::gnssTypeToString(TypeGNSS type) {
    switch(type) {
    case GNSS_GPS:    return "GPS";
    case GNSS_GLONASS: return "GLONASS";
    case GNSS_GALILEO: return "Galileo";
    case GNSS_BEIDU:   return "BeiDou";
    default:           return "Unknown";
    }
}

QString NavigationDataFormatter::formatCoordinate(double value, bool isLatitude) {
    QString direction;
    if (isLatitude) {
        direction = (value >= 0) ? "N" : "S";
    } else {
        direction = (value >= 0) ? "E" : "W";
    }

    value = std::abs(value);
    int degrees = static_cast<int>(value);
    double minutesDecimal = (value - degrees) * 60;
    int minutes = static_cast<int>(minutesDecimal);
    double seconds = (minutesDecimal - minutes) * 60;

    return QString("%1°%2'%3\" %4")
        .arg(degrees)
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 5, 'f', 2, QLatin1Char('0'))
        .arg(direction);
}
