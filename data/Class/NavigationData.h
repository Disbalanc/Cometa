#ifndef NAVIGATIONDATA_H
#define NAVIGATIONDATA_H

#include <QTime>
#include <QDate>
#include <QDataStream>
#include <QMultiMap>

enum ParseResult {
    ERROR = 0,
    OK = 1,
};

// Структура для данных GNRMC
struct GNRMCData {
    enum CoordinateDefinition {
        COORDINATE_AUTO = 0, //Автономный
        COORDINATE_DIFF, //дифференциальный
        COORDINATE_APPROX, //аппроксимация
        COORDINATE_FIX, // фиксированные данные.
        COORDINATE_INVALID, //недостоверные данные.
    };
    int id;
    QTime time; // Время UTC в формате "ЧЧММССС.МСМСМС".
    bool isValid; //Достоверность полученных координат. 'A' - данные достоверны. 'V' - ошибочные данные.
    double latitude; // Широта, градусы, формат XX.XXXX (N-Положительное, S-Отрицательное)
    double longitude; // Долгота, градусы, формат XX.XXXX (E-Положительное, W-Отрицательное)
    double speed; // скорость, м/с
    double course; // Курс на истинный полюс в градусах.
    QDate date; //Дата UTC в формате "ДД.ММ.ГГГГ".
    float magnDeviation; // Магнитное склонение в градусах.
    CoordinateDefinition coordinateDefinition;
    bool statusNav; //Способ вычисления координат
    ParseResult result; //результат парсинга

    friend QDataStream& operator>>(QDataStream& in, GNRMCData& data);
};

// Структура для данных GNGGA
struct GNGGAData {
    enum AltitudeUnits {
        METER = 0, //метры
        FOOT, //футы
    };
    enum CoordinateDefinition {
        COORDINATE_UNDEFINE = 0, //недоступно.
        COORDINATE_AUTO, //автономно.
        COORDINATE_DIFF, //дифференциально.
        COORDINATE_PPS, //PPS.
        COORDINATE_FIX_RTK, //фиксированный RTK.
        COORDINATE_NO_FIX_RTK, //не фиксированный RTK.
        COORDINATE_EXTRAPOL, //экстраполяция.
        COORDINATE_FIX_COORD, //фиксированные координаты.
        COORDINATE_SIMULATION, //режим симуляции.
    };
    QTime time; // Время UTC в формате "ЧЧММССС.МСМСМС".
    double latitude; // Широта, градусы, формат XX.XXXX (N-Положительное, S-Отрицательное)
    double longitude; // Долгота, градусы, формат XX.XXXX (E-Положительное, W-Отрицательное)
    CoordinateDefinition coordDef; //Способ вычисления координат
    int satellitesCount; // Количество активных спутников, от "00" до "12".
    uint32_t HDOP; // Горизонтальный геометрический фактор ухудшения точности *10
    float altitude; // Высота над уровнем моря (geoid), единицы измерения высоты.
    AltitudeUnits altUnit;
    float diffElipsoidSeaLevel;//Разница между эллипсоидом земли и уровнем моря (geoid), единицы измерения.
    AltitudeUnits diffElipsUnit;
    uint32_t countSecDGPS; //Количество секунд прошедших с получения последней DGPS поправки (SC104).
    int idDGPS; //ID базовой станции предоставляющей DGPS поправки (если включено DGPS).
    ParseResult result;

    friend QDataStream& operator>>(QDataStream& in, GNGGAData& data);
};

enum TypeFormat {
    FORMAT_NO_DATA = 0, //нет решения
    FORMAT_2D, //2D
    FORMAT_3D, //3D
};
enum TypeGNSS {
    GNSS_GPS = 0, //GPS
    GNSS_GLONASS, //GLONASS
    GNSS_GALILEO, //GALILEO
    GNSS_BEIDU, //BEIDU
};
// Структура для данных GNGSA
struct GNGSAData {
    bool isAuto; //Режим выбора формата 2D/3D: 'A'-автоматический / 'M'-ручной.
    TypeFormat typeFormat; //Режим выбранного формата '1'-нет решения / '2'-2D / '3'-3D.
    int satellitesUsedId[12]; // ID активных спутников.
    uint32_t PDOP; // Пространственный геометрический фактор ухудшения точности *10
    uint32_t HDOP; // Горизонтальный геометрический фактор ухудшения точности * 10
    uint32_t VDOP; // Вертикальный геометрический фактор ухудшения точности *10
    TypeGNSS typeGNSS; //Номер навигационной системы
    ParseResult result; //Результат парсинга

    friend QDataStream& operator>>(QDataStream& in, GNGSAData& data);
};
// Структура для данных GNZDA
struct GNZDAData {
    QTime time; // Время UTC в формате "ЧЧММССС.МСМСМС".
    QDate date; // Дата UTC в формате "ДД.ММ.ГГГГ".
    uint32_t localOffset; // Локальный сдвиг
    ParseResult result; //Результат парсинга

    friend QDataStream &operator>>(QDataStream &in, GNZDAData &data);

};
// Структура для данных GNDHV
struct GNDHVData {
    QTime time; // Время UTC
    double speed3D; // Скорость 3D в м/с
    double speedECEF_X; // Скорость ECEF-X в м/с
    double speedECEF_Y; // Скорость ECEF-Y в м/с
    double speedECEF_Z; // Скорость ECEF-Z в м/с
    double speed; // Общая скорость в м/с
    ParseResult result; // Результат парсинга

    friend QDataStream &operator>>(QDataStream &in, GNDHVData &data);

};
// Структура для данных GNGST
struct GNGSTData {
    QTime time; // Время UTC
    double rms; // Среднее квадратическое значение (RMS) стандартной девиации диапазонов в метрах
    double semiMajorError; // Ошибка элипса полуоси semi-major в метрах
    double semiMinorError; // Ошибка элипса полуоси semi-minor в метрах
    double semiMajorOrientation; // Ошибка ориентации элипса полуоси semi-major в градусах
    double latitudeError; // Ошибка широты в метрах
    double longitudeError; // Ошибка долготы в метрах
    double altitudeError; // Ошибка высоты в метрах
    ParseResult result; // Результат парсинга

    friend QDataStream &operator>>(QDataStream &in, GNGSTData &data);

};
// Структура для данных GPTXT
struct GPTXTData {
    int messageCount; // Количество строк с текстом
    int messageNumber; // Номер строки
    int messageType; // Идентификатор типа сообщения
    QString message; // Текстовое сообщение
    ParseResult result; // Результат парсинга

    friend QDataStream &operator>>(QDataStream &in, GPTXTData &data);
};
// Структура для данных GNGLL
struct GNGLLData {
    double latitude; // Широта
    double longitude; // Долгота
    QTime time; // Время UTC
    bool isValid; // Достоверность полученных координат
    ParseResult result; // Результат парсинга

    friend QDataStream &operator>>(QDataStream &in, GNGLLData &data);

};
// Структура для данных спутника
struct SatelliteInfo {
    int prn; // ID спутника
    double elevation; // Угол возвышения
    double azimuth; // Азимут
    double snr; // Соотношение сигнал/шум

    friend QDataStream& operator>>(QDataStream& in, SatelliteInfo& data);
};

// Структура для данных GLGSV
struct GLGSVData {
    int totalMessages; // Общее количество сообщений
    int messageNumber; // Номер сообщения
    int satellitesCount; // Количество наблюдаемых спутников
    QVector<SatelliteInfo> satelliteData; // ID спутника, угол возвышения, азимут, уровень сигнала
    ParseResult result; // Результат парсинга

    friend QDataStream& operator>>(QDataStream& in, GLGSVData& data);
};
// Структура для данных GNVTG
struct GNVTGData {
    double trueCourse; // Курс на истинный полюс в градусах
    double magneticCourse; // Магнитный курс
    double speedKnots; // Скорость в узлах
    double speedKmh; // Скорость в км/ч
    bool isValid; // Достоверность данных
    ParseResult result; // Результат парсинга

    friend QDataStream &operator>>(QDataStream &in, GNVTGData &data);
};
enum MsgType{
    GNRMC=0,
    GNGGA,
    GNGSA,
    GNZDA,
    GNDHV,
    GNGST,
    GPTXT,
    GNGLL,
    GLGSV,
    GNVTG
};

// Основная структура для навигационных данных
struct NavigationData {
    int id; //индифакационный номер
    QDateTime timestamp; //время парсинга
    ParseResult result; //результат парсинга
    MsgType type; // тип
    int size; // размер строки
    QByteArray data; // обобщенные данные

    struct DeserializedData {
        GNRMCData gnrmc;
        GNGGAData gngga;
        GNZDAData gnzda;
        QDateTime timestamp; //время парсинга
        int id;
    };

    DeserializedData deserialize() const {
        DeserializedData result;
        QDataStream stream(data);

        stream >> result.id
            >> result.timestamp
            >> result.gnzda.date
            >> result.gnzda.time
            >> result.gnrmc.isValid
            >> result.gngga.altitude
            >> result.gnrmc.latitude
            >> result.gnrmc.longitude
            >> result.gnrmc.speed
            >> result.gnrmc.course;

        return result;
    }
};

// Структура для данных карты
struct NavigationDataMap {
    int id;
    QDateTime timestamp; //время парсинга
    QDate date;
    QTime time;
    bool isValid;
    double latitude;
    double longitude;
    float altitude;
    double speed;
    double course;
};

struct NavigationDataTable {
    int id;
    QDateTime timestamp;
    QVariantMap customData ; // Для хранения произвольных данных из разных таблиц
};

// Объявления операторов ввода
QDataStream &operator>>(QDataStream &in, GNRMCData::CoordinateDefinition &coordDef);
QDataStream &operator>>(QDataStream &in, GNRMCData &data);
QDataStream &operator>>(QDataStream &in, GNGGAData::AltitudeUnits &altUnit);
QDataStream &operator>>(QDataStream &in, ParseResult &result);
QDataStream &operator>>(QDataStream &in, GNGGAData &data);
QDataStream &operator>>(QDataStream &in, TypeGNSS &gnss);
QDataStream &operator>>(QDataStream &in, TypeFormat &format);
QDataStream &operator>>(QDataStream &in, GNGSAData &data);
QDataStream &operator>>(QDataStream &in, GNZDAData &data);
QDataStream &operator>>(QDataStream &in, GNDHVData &data);
QDataStream &operator>>(QDataStream &in, GNGSTData &data);
QDataStream &operator>>(QDataStream &in, GPTXTData &data);
QDataStream &operator>>(QDataStream &in, GNGLLData &data);
QDataStream &operator>>(QDataStream &in, SatelliteInfo &data);
QDataStream &operator>>(QDataStream &in, GLGSVData &data);
QDataStream &operator>>(QDataStream &in, GNVTGData &data);

#endif // NAVIGATIONDATA_H
