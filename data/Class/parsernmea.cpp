// parsernmea.cpp
#include "parsernmea.h"
#include <QDateTime>
#include <QDebug>
#include <cmath>

// Константы для валидации
namespace {
constexpr int MAX_LINE_LENGTH = 82;
constexpr int BUFFER_SIZE = 1024;
constexpr double KNOTS_TO_KMH = 1.852;
}

ParserNMEA::ParserNMEA(QObject *parent) : QObject(parent)
{
    inCompleteLine.reserve(BUFFER_SIZE);
    m_buffer.reserve(BUFFER_SIZE);
    m_parsers = {
        {"GNRMC", &ParserNMEA::parseGNRMC},
        {"GNGGA", &ParserNMEA::parseGNGGA},
        {"GNGSA", &ParserNMEA::parseGNGSA},
        {"GNZDA", &ParserNMEA::parseGNZDA},
        {"GNDHV", &ParserNMEA::parseGNDHV},
        {"GNGST", &ParserNMEA::parseGNGST},
        {"GPTXT", &ParserNMEA::parseGPTXT},
        {"GNGLL", &ParserNMEA::parseGNGLL},
        {"GLGSV", &ParserNMEA::parseGLGSV},
        {"GNVTG", &ParserNMEA::parseGNVTG}
    };
}

void ParserNMEA::setLogger(Logger* logger){
    m_logger = logger;
}

NavigationData ParserNMEA::parseData(QString &line)
{
    NavigationData result;
    result.result = ParseResult::ERROR;
    result.timestamp = QDateTime::currentDateTime();

    try {
        const int starPos = line.indexOf('*');
        if (starPos == -1 || starPos + 3 > line.length()) {
            logError("PARSE", QString("Checksum marker not found: %1").arg(line));
        }

        if(!isValidString(line)){
            inCompleteLine += line;
            if (starPos == -1 || starPos + 3 > line.length()) {
                logError("PARSE", QString("Checksum marker not found: %1").arg(line));
            }
            if(!isValidString(inCompleteLine)){
                logError("PARSE", QString("String no type: %1").arg(line));
                return result;
            }
            line = inCompleteLine;
        }

        // Разделение на части
        QVector<QStringRef> parts;
        parts.reserve(20);

        int start = 1;
        const int endPos = starPos;
        const int lineLength = line.length();

        for (int i = start; i < endPos && i < lineLength; ++i) {
            if (line[i] == ',') {
                parts.append(QStringRef(&line, start, i - start));
                start = i + 1;
            }
        }
        parts.append(QStringRef(&line, start, endPos - start));

        if (parts.isEmpty()) {
            throw std::invalid_argument("No message parts");
        }

        // Определение типа сообщения
        const QString msgType = parts.first().toString();
        const auto parser = m_parsers.constFind(msgType);

        if (parser == m_parsers.constEnd()) {
            throw std::invalid_argument(QString("Unsupported message type: %1").arg(msgType).toStdString());
        }

        // Вызов обработчика
        parser.value()(this, parts, result);
        result.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed message");

    } catch (const std::exception &e) {
        logError("PARSE", QString("Error: %1. Data: %2").arg(e.what(), line.left(50)));
        m_buffer = line;
    }

    return result;
}

bool ParserNMEA::isValidString(QString &line)
{
    const int starPos = line.indexOf('*');

    // Проверки
    if (line.length() < 7 || line[0] != '$') {
        logError("PARSE","Invalid message start");
        return false;
    }
    if (starPos == -1 || starPos + 3 > line.length()) {
        logError("PARSE","Checksum marker not found");
        return false;
    }
    if (line.length() > MAX_LINE_LENGTH) {
        logError("PARSE","Message too long");
        return false;
    }
    if (!validateChecksum(line)) {
        logError("PARSE","Checksum validation failed");
        return false;
    }
    return true;
}

// Вспомогательные методы
bool ParserNMEA::validateChecksum(const QString &line) const
{
    const int starPos = line.indexOf('*');
    if (starPos < 1 || starPos + 2 >= line.length()) return false;

    uint8_t calculated = 0;
    const QChar* data = line.constData() + 1; // Пропускаем $

    for (int i = 1; i < starPos; ++i) {
        calculated ^= data[i-1].toLatin1();
    }

    bool ok;
    const uint8_t expected = line.midRef(starPos+1, 2).toInt(&ok, 16);

    if (!ok) {
        logError("CHECKSUM", "Invalid checksum format");
        return false;
    }

    return calculated == expected;
}

void ParserNMEA::parseGNRMC(const QVector<QStringRef> &parts, NavigationData &data) {
    if (parts.size() < 12) {
        logError("GNRMC", "Insufficient parts");
        return;
    }

    GNRMCData rmc;
    const QString msgType = "GNRMC";

    try {
        rmc.time = parseTime(parts[1], msgType);
        rmc.isValid = (parts[2] == "A");
        rmc.latitude = parseCoordinate(parts[3], parts[4], msgType);
        rmc.longitude = parseCoordinate(parts[5], parts[6], msgType);
        rmc.speed = validateRange(parts[7].toDouble(), 0.0, 102.3, "Speed", msgType);
        rmc.course = validateRange(parts[8].toDouble(), 0.0, 360.0, "Course", msgType);
        rmc.date = parseDate(parts[9], msgType);

        // Обработка магнитного отклонения (части 10 и 11)
        if (parts.size() > 10 && !parts[10].isEmpty()) {
            rmc.magnDeviation = parts[10].toDouble();
        } else {
            rmc.magnDeviation = 0.0;
        }

        // Обработка определения координат (часть 11)
        if (parts.size() > 11) {
            int coordDef = parts[11].toInt();
            if (coordDef < 0 || coordDef > GNRMCData::COORDINATE_INVALID) {
                coordDef = GNRMCData::COORDINATE_INVALID;
            }
            rmc.coordinateDefinition = static_cast<GNRMCData::CoordinateDefinition>(coordDef);
        } else {
            rmc.coordinateDefinition = GNRMCData::COORDINATE_INVALID;
        }

        // Обработка статуса навигации (часть 12)
        rmc.statusNav = (parts.size() > 12) ? (parts[12] == "A") : false;

        rmc.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed RMC message");
    } catch (const std::exception& e) {
        rmc.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNRMC;
    data.data = serialize(rmc);
}

// Парсинг сообщения GNGGA
void ParserNMEA::parseGNGGA(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 15) {
        throw std::invalid_argument("GNGGA requires at least 15 fields");
    }

    GNGGAData gga;
    const QString msgType = "GNGGA";

    try {
        // 1. Время UTC
        gga.time = parseTime(parts[1], msgType);

        // 2. Широта
        gga.latitude = parseCoordinate(parts[2], parts[3], msgType);

        // 3. Долгота
        gga.longitude = parseCoordinate(parts[4], parts[5], msgType);

        // 4. Качество фиксации (преобразуем в CoordinateDefinition)
        int fixQuality = parseInt(parts[6], "FixQuality", msgType);
        if (fixQuality < 0 || fixQuality > 8) {
            throw std::out_of_range("Invalid fix quality value");
        }
        gga.coordDef = static_cast<GNGGAData::CoordinateDefinition>(fixQuality);

        // 5. Количество спутников
        gga.satellitesCount = validateRange(
            parseInt(parts[7], "Satellites", msgType), 0, 99, "Satellites", msgType
            );

        // 6. HDOP
        gga.HDOP = static_cast<uint32_t>(
            validateRange(parseDouble(parts[8], "HDOP", msgType), 0.0, 99.9, "HDOP", msgType) * 10
            );

        // 7. Высота и единицы измерения
        if (!parts[9].isEmpty()) {
            gga.altitude = parseDouble(parts[9], "Altitude", msgType);
        } else {
            gga.altitude = 0.0f;
            logWarning(msgType, "Altitude field is empty, using default 0.0");
        }
        // Добавить проверку на inf/nan
        if (std::isinf(gga.altitude) || std::isnan(gga.altitude)) {
            gga.altitude = 0.0f;
            logWarning(msgType, "Invalid altitude value, reset to 0.0");
        }
        gga.altUnit = (parts[10].toString() == "M") ?
                          GNGGAData::METER : GNGGAData::FOOT;
        qDebug()<<gga.altitude;
        // 8. Разница геоида
        gga.diffElipsoidSeaLevel = parseDouble(parts[11], "GeoidSep", msgType);
        gga.diffElipsUnit = (parts[12].toString() == "M") ?
                                GNGGAData::METER : GNGGAData::FOOT;

        // 9. Время с последнего DGPS обновления
        if (!parts[13].isEmpty()) {
            gga.countSecDGPS = static_cast<uint32_t>(
                parseDouble(parts[13], "DGPSAge", msgType) * 10
                );
        }

        // 10. ID станции DGPS
        if (!parts[14].isEmpty()) {
            gga.idDGPS = parseInt(parts[14], "DGPSId", msgType);
        }

        gga.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed GGA message");
    }
    catch (const std::exception& e) {
        gga.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNGGA;
    data.data = serialize(gga);
}

// Парсинг сообщения GNGSA
void ParserNMEA::parseGNGSA(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 18) {
        throw std::invalid_argument("GNGSA requires at least 18 fields");
    }

    GNGSAData gsa;
    const QString msgType = "GNGSA";

    try {
        // 1. Режим автоматического выбора
        gsa.isAuto = (parts[1].toString() == "A");

        // 2. Тип фиксации
        int fixType = parseInt(parts[2], "FixType", msgType);
        if (fixType < 0 || fixType > 3) {
            throw std::out_of_range("Invalid fix type value");
        }
        gsa.typeFormat = static_cast<TypeFormat>(fixType);

        // 3. Список используемых спутников
        for (int i = 0; i < 12; ++i) {
            QStringRef prn = parts[3 + i];
            gsa.satellitesUsedId[i] = prn.isEmpty() ? 0 : prn.toInt();
        }

        // 4. Показатели точности
        gsa.PDOP = static_cast<uint32_t>(
            validateRange(parseDouble(parts[15], "PDOP", msgType), 0.0, 99.9, "PDOP", msgType) * 10
            );
        gsa.HDOP = static_cast<uint32_t>(
            validateRange(parseDouble(parts[16], "HDOP", msgType), 0.0, 99.9, "HDOP", msgType) * 10
            );
        gsa.VDOP = static_cast<uint32_t>(
            validateRange(parseDouble(parts[17], "VDOP", msgType), 0.0, 99.9, "VDOP", msgType) * 10
            );

        // 5. Тип навигационной системы (опциональное поле)
        if (parts.size() > 18 && !parts[18].isEmpty()) {
            int gnssType = parseInt(parts[18], "GNSS", msgType);
            // В методе parseGNGSA:
            if (gnssType < 0 || gnssType > 4) { // Добавить поддержку значения 4
                logWarning(msgType, "Unknown GNSS type, using GPS");
                gnssType = GNSS_GPS;
            }
            gsa.typeGNSS = static_cast<TypeGNSS>(gnssType);
        }

        gsa.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed GSA message");
    }
    catch (const std::exception& e) {
        gsa.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNGSA;
    data.data = serialize(gsa);
}

// Парсинг сообщения GNZDA
void ParserNMEA::parseGNZDA(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 7) {
        throw std::invalid_argument("GNZDA requires at least 7 fields");
    }

    GNZDAData zda;
    const QString msgType = "GNZDA";

    try {
        // 1. Парсинг времени
        zda.time = parseTime(parts[1], msgType);

        // 2. Парсинг даты
        zda.date = parseDate(parts[2], parts[3], parts[4], msgType);

        // 3. Локальный временной сдвиг
        int hours = 0;
        int minutes = 0;

        if (!parts[5].isEmpty()) {
            hours = parseInt(parts[5], "LocalHours", msgType);
        }

        if (!parts[6].isEmpty()) {
            minutes = parseInt(parts[6], "LocalMinutes", msgType);
        }

        // Проверка корректности значений
        hours = qBound(-23, hours, 23);
        minutes = qBound(0, minutes, 59);

        // Сохраняем как количество минут смещения
        zda.localOffset = static_cast<uint32_t>((hours * 60) + minutes);
        //qDebug() << zda.time << zda.date << zda.localOffset;

        zda.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed ZDA message");
    }
    catch (const std::exception& e) {
        zda.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNZDA;
    data.data = serialize(zda);
}

// Парсинг сообщения GNDHV
void ParserNMEA::parseGNDHV(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 7) {
        throw std::invalid_argument("GNDHV requires at least 7 fields");
    }

    GNDHVData dhv;
    const QString msgType = "GNDHV";

    try {
        // 1. Парсинг времени
        dhv.time = parseTime(parts[1], msgType);

        // 2. Скорость 3D
        dhv.speed3D = validateRange(
            parseDouble(parts[2], "Speed3D", msgType), 0.0, 9999.9, "Speed3D", msgType
            );

        // 3. Скорости по осям ECEF
        dhv.speedECEF_X = parseDouble(parts[3], "SpeedX", msgType);
        dhv.speedECEF_Y = parseDouble(parts[4], "SpeedY", msgType);
        dhv.speedECEF_Z = parseDouble(parts[5], "SpeedZ", msgType);

        // 4. Общая скорость
        dhv.speed = validateRange(
            parseDouble(parts[6], "Speed", msgType), 0.0, 9999.9, "Speed", msgType
            );

        // 5. Проверка согласованности данных
        if (std::abs(dhv.speed3D - std::hypot(dhv.speedECEF_X, dhv.speedECEF_Y, dhv.speedECEF_Z)) > 0.1) {
            logWarning(msgType, "3D speed mismatch with ECEF components");
        }

        dhv.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed DHV message");
    }
    catch (const std::exception& e) {
        dhv.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNDHV;
    data.data = serialize(dhv);
}

// Парсинг сообщения GNGST
void ParserNMEA::parseGNGST(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 9) {
        throw std::invalid_argument("GNGST requires at least 9 fields");
    }

    GNGSTData gst;
    const QString msgType = "GNGST";

    try {
        // 1. Парсинг времени
        gst.time = parseTime(parts[1], msgType);

        // 2. RMS стандартной девиации
        gst.rms = validateNonNegative(
            parseDouble(parts[2], "RMS", msgType), "RMS", msgType
            );

        // 3. Ошибки эллипса
        gst.semiMajorError = validateNonNegative(
            parseDouble(parts[3], "SemiMajor", msgType), "SemiMajor", msgType
            );
        gst.semiMinorError = validateNonNegative(
            parseDouble(parts[4], "SemiMinor", msgType), "SemiMinor", msgType
            );
        gst.semiMajorOrientation = validateAngle(
            parseDouble(parts[5], "Orientation", msgType), "Orientation", msgType
            );

        // 4. Ошибки координат
        gst.latitudeError = validateNonNegative(
            parseDouble(parts[6], "LatError", msgType), "LatError", msgType
            );
        gst.longitudeError = validateNonNegative(
            parseDouble(parts[7], "LonError", msgType), "LonError", msgType
            );
        gst.altitudeError = validateNonNegative(
            parseDouble(parts[8], "AltError", msgType), "AltError", msgType
            );

        // 5. Дополнительная проверка согласованности
        if (gst.semiMajorError < gst.semiMinorError) {
            logWarning(msgType, "Major axis smaller than minor axis - possible data corruption");
        }

        gst.result = ParseResult::OK;
        logInfo(msgType, "Successfully parsed GST message");
    }
    catch (const std::exception& e) {
        gst.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNGST;
    data.data = serialize(gst);
}

// В файл parsernmea.cpp добавить:

// Парсинг сообщения GPTXT
void ParserNMEA::parseGPTXT(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 5) {
        throw std::invalid_argument("GPTXT requires at least 5 fields");
    }

    GPTXTData txt;
    const QString msgType = "GPTXT";

    try {
        // 1. Количество сообщений
        txt.messageCount = parseInt(parts[1], "TotalMsgs", msgType);

        // 2. Номер текущего сообщения
        txt.messageNumber = parseInt(parts[2], "MsgNum", msgType);

        // 3. Тип сообщения
        txt.messageType = parseInt(parts[3], "MsgType", msgType);

        // 4. Текстовое сообщение
        txt.message = parts[4].toString();

        // 5. Проверка согласованности
        if (txt.messageNumber < 1 || txt.messageNumber > txt.messageCount) {
            throw std::out_of_range("Message number out of range");
        }

        txt.result = ParseResult::OK;
        logInfo(msgType, QString("Parsed text message %1/%2: %3")
                             .arg(txt.messageNumber)
                             .arg(txt.messageCount)
                             .arg(txt.message.left(50)));
    }
    catch (const std::exception& e) {
        txt.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GPTXT;
    data.data = serialize(txt);
}

// Парсинг сообщения GNGLL
void ParserNMEA::parseGNGLL(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 7) {
        throw std::invalid_argument("GNGLL requires at least 7 fields");
    }

    GNGLLData gll;
    const QString msgType = "GNGLL";

    try {
        // 1. Парсинг координат
        gll.latitude = parseCoordinate(parts[1], parts[2], msgType);
        gll.longitude = parseCoordinate(parts[3], parts[4], msgType);

        // 2. Парсинг времени
        gll.time = parseTime(parts[5], msgType);

        // 3. Статус валидности
        QString status = parts[6].toString();
        if (status == "A") {
            gll.isValid = true;
        } else if (status == "V") {
            gll.isValid = false;
        } else {
            throw std::invalid_argument("Invalid data status");
        }

        // 4. Дополнительная проверка (опциональные поля)
        if (parts.size() > 7) {
            QString mode = parts[7].toString();
            if (!mode.isEmpty() && mode != "A" && mode != "D" && mode != "E" && mode != "N" && mode != "S") {
                logWarning(msgType, "Invalid mode indicator");
            }
        }

        gll.result = ParseResult::OK;
        logInfo(msgType, QString("Parsed position: %1, %2")
                             .arg(gll.latitude, 0, 'f', 6)
                             .arg(gll.longitude, 0, 'f', 6));
    }
    catch (const std::exception& e) {
        gll.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNGLL;
    data.data = serialize(gll);
}

// Парсинг сообщения GLGSV
void ParserNMEA::parseGLGSV(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 4) {
        throw std::invalid_argument("GLGSV requires at least 4 fields");
    }

    GLGSVData gsv;
    const QString msgType = "GLGSV";

    try {
        // 1. Общая информация
        gsv.totalMessages = parseInt(parts[1], "TotalMsgs", msgType);
        gsv.messageNumber = parseInt(parts[2], "MsgNum", msgType);
        gsv.satellitesCount = parseInt(parts[3], "Satellites", msgType);

        // 2. Проверка согласованности
        if (gsv.messageNumber < 1 || gsv.messageNumber > gsv.totalMessages) {
            throw std::out_of_range("Message number out of range");
        }

        // 3. Парсинг данных спутников
        int satDataFields = (parts.size() - 4) / 4;
        for (int i = 0; i < satDataFields; ++i) {
            int idx = 4 + i * 4;

            SatelliteInfo info;
            info.prn = parseInt(parts[idx], "PRN", msgType);
            info.elevation = parseDouble(parts[idx+1], "Elevation", msgType);
            info.azimuth = parseDouble(parts[idx+2], "Azimuth", msgType);
            info.snr = parts.size() > idx+3 ?
                           parseDouble(parts[idx+3], "SNR", msgType) : 0.0;

            // Проверка допустимых значений
            info.elevation = validateRange(info.elevation, 0.0, 90.0, "Elevation", msgType);
            info.azimuth = validateAngle(info.azimuth, "Azimuth", msgType);
            info.snr = validateRange(info.snr, 0.0, 99.0, "SNR", msgType);

            gsv.satelliteData.append(info);
        }

        // 4. Проверка общего количества спутников
        if (gsv.messageNumber == gsv.totalMessages &&
            gsv.satellitesCount != gsv.satelliteData.size()) {
            logWarning(msgType, "Satellite count mismatch");
        }

        gsv.result = ParseResult::OK;
        logInfo(msgType, QString("Parsed %1 satellites in message %2/%3")
                             .arg(gsv.satelliteData.size())
                             .arg(gsv.messageNumber)
                             .arg(gsv.totalMessages));
    }
    catch (const std::exception& e) {
        gsv.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GLGSV;
    data.data = serialize(gsv);
}

// Парсинг сообщения GNVTG
void ParserNMEA::parseGNVTG(const QVector<QStringRef>& parts, NavigationData& data)
{
    if (parts.size() < 9) {
        throw std::invalid_argument("GNVTG requires at least 9 fields");
    }

    GNVTGData vtg;
    const QString msgType = "GNVTG";

    try {
        // 1. Парсинг курсов
        vtg.trueCourse = validateAngle(
            parseDouble(parts[1], "TrueCourse", msgType),
            "TrueCourse",
            msgType
            );

        vtg.magneticCourse = validateAngle(
            parseDouble(parts[3], "MagneticCourse", msgType),
            "MagneticCourse",
            msgType
            );

        // 2. Парсинг скоростей
        vtg.speedKnots = validateRange(
            parseDouble(parts[5], "SpeedKnots", msgType),
            0.0,
            999.9,
            "SpeedKnots",
            msgType
            );

        vtg.speedKmh = validateRange(
            parseDouble(parts[7], "SpeedKmh", msgType),
            0.0,
            9999.9,
            "SpeedKmh",
            msgType
            );

        // 3. Проверка валидности данных
        QString mode = parts.size() > 8 ? parts[8].toString() : "";
        vtg.isValid = (mode == "A"); // A - Autonomous mode

        // 4. Дополнительная проверка согласованности
        double convertedKnots = vtg.speedKmh / 1.852;
        if (std::abs(vtg.speedKnots - convertedKnots) > 0.1) {
            logWarning(msgType, "Speed units mismatch");
        }

        vtg.result =ParseResult::OK;
        logInfo(msgType, QString("Parsed course %1°, speed %2 knots")
                             .arg(vtg.trueCourse, 0, 'f', 1)
                             .arg(vtg.speedKnots, 0, 'f', 1));
    }
    catch (const std::exception& e) {
        vtg.result = ParseResult::ERROR;
        logError(msgType, QString("Parse error: %1").arg(e.what()));
        throw;
    }

    data.type = MsgType::GNVTG;
    data.data = serialize(vtg);
}

QTime ParserNMEA::parseTime(const QStringRef &ref, const QString &msgType) const
{
    if (ref .length() < 6) {
        logError(msgType, "Invalid time format");
        return QTime();
    }

    return QTime(
        ref.mid(0, 2).toInt(),
        ref.mid(2, 2).toInt(),
        ref.mid(4, 2).toInt(),
        ref.length() > 6 ? ref.mid(6).toInt() * 10 : 0
        );
}

QDate ParserNMEA::parseDate(const QStringRef& dayRef,
                            const QStringRef& monthRef,
                            const QStringRef& yearRef,
                            const QString& msgType) const
{
    bool ok;
    int day = dayRef.toInt(&ok);
    if (!ok || day < 1 || day > 31) {
        logError(msgType, "Invalid day value");
        return QDate();
    }

    int month = monthRef.toInt(&ok);
    if (!ok || month < 1 || month > 12) {
        logError(msgType, "Invalid month value");
        return QDate();
    }

    int year = yearRef.toInt(&ok);
    if (!ok || year < 1970 || year > 2100) {
        logError(msgType, "Invalid year value");
        return QDate();
    }

    return QDate(year, month, day);
}

QDate ParserNMEA::parseDate(const QStringRef &ref, const QString& msgType) const {
    if (ref.length() != 6) {
        logError(msgType, "Invalid date format, expected 6 characters");
        return QDate(); // Возвращаем пустую дату в случае ошибки
    }

    // Извлечение дня, месяца и года
    int day = ref.mid(0, 2).toInt();
    int month = ref.mid(2, 2).toInt();
    int year = ref.mid(4, 2).toInt() + 2000; // Предполагаем, что год в формате YY

    // Проверка на корректность значений
    if (day < 1 || day > 31 || month < 1 || month > 12) {
        logError(msgType, QString("Invalid day (%1) or month (%2)").arg(day).arg(month));
        return QDate(); // Возвращаем пустую дату в случае ошибки
    }

    // Создание объекта QDate
    QDate date(year, month, day);
    if (!date.isValid()) {
        logError(msgType, "Constructed date is invalid");
        return QDate(); // Возвращаем пустую дату в случае ошибки
    }

    return date; // Возвращаем валидную дату
}

QTime ParserNMEA::parseTimeWithMilliseconds(const QStringRef& timeRef, const QString& msgType) const
{
    if (timeRef.length() < 6) {
        logError(msgType, "Invalid time format");
        return QTime();
    }

    QString timeStr = timeRef.toString();
    timeStr.replace('.', ':');

    QStringList parts = timeStr.split(':');
    if (parts.size() < 3) {
        logError(msgType, "Time format missing milliseconds");
        return QTime();
    }

    bool ok;
    int hours = parts[0].left(2).toInt(&ok);
    if (!ok || hours < 0 || hours > 23) {
        logError(msgType, "Invalid hours in time");
        return QTime();
    }

    int minutes = parts[0].mid(2, 2).toInt(&ok);
    if (!ok || minutes < 0 || minutes > 59) {
        logError(msgType, "Invalid minutes in time");
        return QTime();
    }

    int seconds = parts[1].left(2).toInt(&ok);
    if (!ok || seconds < 0 || seconds > 59) {
        logError(msgType, "Invalid seconds in time");
        return QTime();
    }

    int msec = parts[2].left(2).toInt(&ok);
    if (!ok || msec < 0 || msec > 99) {
        logError(msgType, "Invalid milliseconds in time");
        return QTime();
    }

    return QTime(hours, minutes, seconds, msec);
}

double ParserNMEA::parseDouble(const QStringRef& ref,
                               const QString& fieldName,
                               const QString& msgType) const
{
    bool ok;
    double value = ref.toDouble(&ok);
    if (!ok) {
        logError(msgType, QString("Invalid double value for %1").arg(fieldName));
        return 0.0;
    }
    return value;
}

int ParserNMEA::parseInt(const QStringRef& ref,
                         const QString& fieldName,
                         const QString& msgType) const
{
    bool ok;
    int value = ref.toInt(&ok);
    if (!ok) {
        logError(msgType, QString("Invalid integer value for %1").arg(fieldName));
        return 0;
    }
    return value;
}

QString ParserNMEA::parseString(const QStringRef &ref, const QString &fieldName, const QString &msgType) const
{
    if (ref.isEmpty()) {
        logError(msgType, QString("Empty value for %1").arg(fieldName));
        return QString();
    }
    return ref.toString();
}

double ParserNMEA::parseCoordinate(const QStringRef& coord,
                                   const QStringRef& dir,
                                   const QString& msgType) const
{
    if (coord.isEmpty() || dir.isEmpty()) {
        logError(msgType, "Empty coordinate field");
        return 0.0;
    }

    bool ok;
    double value = coord.toDouble(&ok);
    if (!ok) {
        logError(msgType, "Invalid coordinate format");
        return 0.0;
    }

    double degrees = static_cast<int>(value / 100);
    double minutes = value - degrees * 100;
    double result = degrees + minutes / 60.0;

    if (dir == "S" || dir == "W") {
        result *= -1;
    }

    return result;
}

double ParserNMEA::validateRange(double value, double min, double max,
                                 const QString& field, const QString& context) const
{
    if (value < min || value > max) {
        logError(context, QString("Value %1 out of range [%2-%3] for %4")
                              .arg(value).arg(min).arg(max).arg(field));
        return qBound(min, value, max);
    }
    return value;
}

int ParserNMEA::validateRange(int value, int min, int max, const QString &fieldName, const QString &msgType) const
{
    if (value < min || value > max) {
        logError(msgType, QString("Value out of range for %1: %2").arg(fieldName).arg(value));
    }
    return qBound(min, value, max);
}

double ParserNMEA::validateAngle(double degrees,
                                 const QString& field,
                                 const QString& context) const
{
    degrees = fmod(degrees, 360.0);
    if (degrees < 0.0) degrees += 360.0;

    if (degrees < 0.0 || degrees >= 360.0) {
        logError(context, QString("Invalid angle %1 for %2").arg(degrees).arg(field));
        throw std::out_of_range("Angle out of valid range");
    }
    return degrees;
}

double ParserNMEA::validateNonNegative(double value,
                                       const QString& field,
                                       const QString& context) const
{
    if (value < 0.0) {
        logError(context, QString("Negative value %1 for %2").arg(value).arg(field));
        throw std::out_of_range("Negative value not allowed");
    }
    return value;
}

QByteArray ParserNMEA::serialize(const GNRMCData &data) {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    try {
        stream << data.time
               << data.isValid
               << data.latitude
               << data.longitude
               << data.speed
               << data.course
               << data.date
               << data.magnDeviation
               << static_cast<int>(data.coordinateDefinition)
               << data.statusNav;
    } catch (const std::exception& e) {
        logError("SERIALIZE", QString("GNRMC serialization failed: %1").arg(e.what()));
        return QByteArray();
    }
    return buffer;
}

QByteArray ParserNMEA::serialize(const GNGGAData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    // Записываем каждое поле с проверкой
    try {
        stream << data.time
               << data.latitude
               << data.longitude
               << static_cast<int>(data.coordDef)
               << data.satellitesCount
               << data.HDOP
               << data.altitude  // Записывается как double
               << (data.altUnit == GNGGAData::METER ? "M" : "F")
               << data.diffElipsoidSeaLevel
               << (data.diffElipsUnit == GNGGAData::METER ? "M" : "F")
               << static_cast<double>(data.countSecDGPS) / 10.0
               << data.idDGPS;
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("GGA serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GNGSAData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    try {
        // Записываем основные поля
        stream << data.isAuto
               << static_cast<int>(data.typeFormat);

        // Записываем массив спутников
        for (int i = 0; i < 12; ++i) {
            stream << data.satellitesUsedId[i];
        }

        // Записываем DOP значения
        stream << static_cast<double>(data.PDOP) / 10.0
               << static_cast<double>(data.HDOP) / 10.0
               << static_cast<double>(data.VDOP) / 10.0
               << static_cast<int>(data.typeGNSS);
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("GSA serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GNZDAData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    try {
        // Записываем время с миллисекундами
        stream << data.time;

        // Записываем дату
        stream << data.date;

        // Распаковываем временное смещение
        int hours = static_cast<int>(data.localOffset) / 60;
        int minutes = static_cast<int>(data.localOffset) % 60;

        stream << data.localOffset;
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("ZDA serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GNDHVData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    try {
        // Записываем время
        stream << data.time.toString("HHmmss.zzz");

        // Записываем скорости
        stream << data.speed3D
               << data.speedECEF_X
               << data.speedECEF_Y
               << data.speedECEF_Z
               << data.speed;
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("DHV serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GNGSTData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    try {
        // Записываем время с миллисекундами
        stream << data.time.toString("HHmmss.zzz");

        // Записываем значения ошибок
        stream << data.rms
               << data.semiMajorError
               << data.semiMinorError
               << data.semiMajorOrientation
               << data.latitudeError
               << data.longitudeError
               << data.altitudeError;
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("GST serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GLGSVData& data) {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    try {
        stream << data.totalMessages
               << data.messageNumber
               << data.satellitesCount;

        stream << static_cast<int>(data.satelliteData.size());
        for (const auto& sat : data.satelliteData) {
            stream << sat.prn
                   << sat.elevation
                   << sat.azimuth
                   << sat.snr;
        }
    } catch (const std::exception& e) {
        logError("SERIALIZE", QString("GLGSV serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GPTXTData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_13);

    try {
        stream << data.messageCount
               << data.messageNumber
               << data.messageType
               << data.message;
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("GPTXT serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GNGLLData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    try {
        // Записываем координаты
        stream << data.latitude
               << data.longitude;

        // Записываем время
        stream << data.time.toString("HHmmss.zzz");

        // Статус валидности
        stream << (data.isValid ? "A" : "V");
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("GNGLL serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

QByteArray ParserNMEA::serialize(const GNVTGData& data)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    try {
        // Форматируем данные согласно стандарту NMEA
        stream << QString::number(data.trueCourse, 'f', 1) << "T"
               << QString::number(data.magneticCourse, 'f', 1) << "M"
               << QString::number(data.speedKnots, 'f', 1) << "N"
               << QString::number(data.speedKmh, 'f', 1) << "K"
               << (data.isValid ? "A" : "V");
    }
    catch (const std::exception& e) {
        logError("SERIALIZE", QString("GNVTG serialization failed: %1").arg(e.what()));
        return QByteArray();
    }

    return buffer;
}

void ParserNMEA::logError(const QString &msgType, const QString &message) const
{
    if (m_logger) {
        m_logger->log(Logger::Error, QString("[%1] %2").arg(msgType, message));
    }
}

void ParserNMEA::logWarning(const QString &msgType, const QString &message) const
{
    if (m_logger) {
        m_logger->log(Logger::Warning, QString("[%1] %2").arg(msgType, message));
    }
}

void ParserNMEA::logInfo(const QString &msgType, const QString &message) const
{
    if (m_logger) {
        m_logger->log(Logger::Info, QString("[%1] %2").arg(msgType, message));
    }
}
