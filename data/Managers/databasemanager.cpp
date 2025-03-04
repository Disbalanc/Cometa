#include "databasemanager.h"

DatabaseManager::DatabaseManager(const QString &dbName, QObject *parent) : QObject(parent) {
    QDir().mkpath(QDir::currentPath() + "/database");
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    initializeDatabase();
    qRegisterMetaType<NavigationDataMap>("NavigationDataMap");
    qRegisterMetaType<QList<NavigationDataMap>>("QList<NavigationDataMap>");
}

bool DatabaseManager::initializeDatabase() {
    if (!open()) {
        return false;
    }

    m_tables = {
        {"gnrmc_data",
         "CREATE TABLE IF NOT EXISTS gnrmc_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "time TEXT, "                     // QTime
         "date TEXT, "                     // QDate
         "latitude REAL, "                 // double
         "longitude REAL, "                // double
         "speed REAL, "                    // double
         "course REAL, "                   // double
         "isValid INTEGER, "               // bool
         "magnDeviation REAL, "            // float
         "coordinateDefinition INTEGER, "  // enum CoordinateDefinition
         "statusNav INTEGER, "             // bool
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gngga_data",
         "CREATE TABLE IF NOT EXISTS gngga_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "time TEXT, "                     // QTime
         "latitude REAL, "                // double
         "longitude REAL, "               // double
         "coordDef INTEGER, "             // enum CoordinateDefinition
         "satellitesCount INTEGER, "      // int
         "hdop INTEGER, "                 // uint32_t (храним как целое)
         "altitude REAL, "                // float
         "altUnit INTEGER, "              // enum AltitudeUnits
         "diffElipsoidSeaLevel REAL, "    // float
         "diffElipsUnit INTEGER, "        // enum AltitudeUnits
         "countSecDGPS INTEGER, "         // uint32_t
         "idDGPS INTEGER, "               // int
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gngsa_data",
         "CREATE TABLE IF NOT EXISTS gngsa_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "isAuto INTEGER, "               // bool
         "typeFormat INTEGER, "           // enum TypeFormat
         "satellitesUsed TEXT, "           // JSON array [int]
         "pdop INTEGER, "                 // uint32_t
         "hdop INTEGER, "                 // uint32_t
         "vdop INTEGER, "                 // uint32_t
         "typeGNSS INTEGER, "             // enum TypeGNSS
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"glgsv_data",
         "CREATE TABLE IF NOT EXISTS glgsv_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "totalMessages INTEGER, "         // int
         "messageNumber INTEGER, "         // int
         "satellitesCount INTEGER, "        // int
         "satelliteData TEXT, "             // JSON array of SatelliteInfo
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gnzda_data",
         "CREATE TABLE IF NOT EXISTS gnzda_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "time TEXT, "                     // QTime
         "date TEXT, "                     // QDate
         "localOffset INTEGER, "           // uint32_t (минуты)
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gndhv_data",
         "CREATE TABLE IF NOT EXISTS gndhv_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "time TEXT, "                     // QTime
         "speed3D REAL, "                 // double
         "speedECEF_X REAL, "             // double
         "speedECEF_Y REAL, "             // double
         "speedECEF_Z REAL, "             // double
         "speed REAL, "                   // double
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gngst_data",
         "CREATE TABLE IF NOT EXISTS gngst_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "time TEXT, "                     // QTime
         "rms REAL, "                      // double
         "semiMajorError REAL, "           // double
         "semiMinorError REAL, "           // double
         "semiMajorOrientation REAL, "     // double
         "latitudeError REAL, "            // double
         "longitudeError REAL, "            // double
         "altitudeError REAL, "            // double
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gngll_data",
         "CREATE TABLE IF NOT EXISTS gngll_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "latitude REAL, "                 // double
         "longitude REAL, "                // double
         "time TEXT, "                     // QTime
         "isValid INTEGER, "               // bool
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"gnvtg_data",
         "CREATE TABLE IF NOT EXISTS gnvtg_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "navigation_data_id INTEGER NOT NULL, "
         "trueCourse REAL, "               // double
         "magneticCourse REAL, "           // double
         "speedKnots REAL, "               // double
         "speedKmh REAL, "                 // double
         "isValid INTEGER, "               // bool
         "FOREIGN KEY(navigation_data_id) REFERENCES navigation_data(id))"},

        {"navigation_data",
         "CREATE TABLE IF NOT EXISTS navigation_data ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "flight_name TEXT NOT NULL, "
         "timestamp TEXT NOT NULL, "        // QDateTime
         "FOREIGN KEY(flight_name) REFERENCES flights(flight_name))"},

        {"flights",
         "CREATE TABLE IF NOT EXISTS flights ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "flight_name TEXT NOT NULL UNIQUE, "
         "status TEXT, "
         "createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP)"}
    };

    return createTables(m_tables);
}

void DatabaseManager::setLogger(Logger *logger) {
    m_logger = logger;
}

bool DatabaseManager::open() {
    if (db.isOpen()) {
        return false; // Если база данных уже открыта, просто возвращаем true
    }
    if (!db.open()) {
        return false;
    }
    return true;
}

void DatabaseManager::close() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "База данных закрыта.";
    }
}

bool DatabaseManager::createTables(const QVector<QPair<QString, QString>>& tables)
{
    if (!db.isOpen()) return false;

    QSqlQuery query;
    bool success = true;

    if (!db.transaction()) {
        logError("Failed to start transaction");
        return false;
    }

    for (const auto& [tableName, ddl] : tables) {
        if (!query.exec(ddl)) {
            logQueryError("Create table " + tableName, query);
            success = false;
        }
    }

    if (success) {
        db.commit();
        return true;
    } else {
        db.rollback();
        return false;
    }
}

void DatabaseManager::validateTableStructure(const QString &tableName) {
    QSqlQuery query;
    query.prepare(QString("PRAGMA table_info(%1)").arg(tableName));

    if (!query.exec()) {
        logQueryError(QString("Get schema for %1").arg(tableName), query);
        return;
    }

    QStringList columns;
    while (query.next()) {
        columns << query.value(1).toString();
    }

    if (m_logger) {
        m_logger->log(Logger::Debug,
                      QString("Table %1 columns: %2").arg(tableName).arg(columns.join(", ")));
    }
}

void DatabaseManager::logQueryError(const QString &operation, const QSqlQuery &query) {
    QString error = QString("%1 failed: %2").arg(operation).arg(query.lastError().text());
    if (m_logger) {
        m_logger->log(Logger::Error, error);
    } else {
        qWarning() << error;
    }
}

void DatabaseManager::logQueryDetails(const QSqlQuery &query) {
    if (m_logger) {
        QString details = "Bound values: ";
        QMapIterator<QString, QVariant> it(query.boundValues());
        while (it.hasNext()) {
            it.next();
            details += QString("%1=%2, ").arg(it.key()).arg(it.value().toString());
        }
        m_logger->log(Logger::Debug, details);
    }
}

bool DatabaseManager::executeTimedQuery(QSqlQuery &query) {
    QElapsedTimer timer;
    timer.start();

    bool result = query.exec();

    if (m_logger) {
        m_logger->log(Logger::Debug, QString("Query executed in %1 ms: %2")
                                         .arg(timer.elapsed()).arg(query.executedQuery()));
    }
    return result;
}

void DatabaseManager::logQueryStats() {
    if (m_logger) {
        QString report = QString("Database Statistics:\n"
                                 "Total Queries: %1\n"
                                 "Failed Queries: %2\n")
                             .arg(stats.totalQueries)
                             .arg(stats.failedQueries);
        m_logger->log(Logger::Info, report);
    }
}

bool DatabaseManager::deleteFlight(const QString &flightName)
{
    if (!db.transaction()) {
        logError("Failed to start transaction");
        return false;
    }

    try {
        const QStringList tables = {
            "gnrmc_data", "gngga_data", "gngsa_data", "glgsv_data",
            "gnzda_data", "gndhv_data", "gngst_data", "gngll_data",
            "gnvtg_data", "navigation_data", "flights"
        };

        QSqlQuery query;
        for (const QString &table : tables) {
            QString queryText;
            if (table == "navigation_data" || table == "flights") {
                queryText = QString("DELETE FROM %1 WHERE flight_name = ?").arg(table);
            } else {
                queryText = QString("DELETE FROM %1 WHERE navigation_data_id IN "
                                    "(SELECT id FROM navigation_data WHERE flight_name = ?)").arg(table);
            }

            query.prepare(queryText);
            query.addBindValue(flightName);

            if (!query.exec()) {
                QString errorText = QString("Failed to delete from %1: %2").arg(table).arg(query.lastError().text());
                logError(errorText);
                throw std::runtime_error(errorText.toStdString());
            }
        }

        if (!db.commit()) {
            QString errorText = "Commit failed: " + db.lastError().text();
            logError(errorText);
            throw std::runtime_error(errorText.toStdString());
        }
        return true;
    } catch (const std::exception& e) {
        db.rollback();
        logError(QString("Transaction failed: %1").arg(e.what()));
        return false;
    }
}

bool DatabaseManager::saveNavigationData(const NavigationData &data) {
    QElapsedTimer timer;
    timer.start();

    if (!db.isOpen() && !open()) {
        logError("Database not open");
        return false;
    }

    static QHash<MsgType, QString> typeNames = {
        {MsgType::GNRMC, "GNRMC"}, {MsgType::GNGGA, "GNGGA"},
        {MsgType::GNGSA, "GNGSA"}, {MsgType::GNZDA, "GNZDA"},
        {MsgType::GNDHV, "GNDHV"}, {MsgType::GNGST, "GNGST"},
        {MsgType::GPTXT, "GPTXT"}, {MsgType::GNGLL, "GNGLL"},
        {MsgType::GLGSV, "GLGSV"}, {MsgType::GNVTG, "GNVTG"}
    };

    try {
        if (!db.transaction()) {
            throw std::runtime_error("Failed to start transaction");
        }
        if(firstType.isEmpty()){
            firstType = data.type;
        }

        if (data.type == MsgType::GNRMC) {
            QSqlQuery navQuery;
            navQuery.prepare(
                "INSERT INTO navigation_data "
                "(flight_name, timestamp) "
                "VALUES (?, ?)");

            navQuery.addBindValue(flight_name);
            navQuery.addBindValue(data.timestamp.toString(Qt::ISODateWithMs));

            if (!navQuery.exec()) {
                throw std::runtime_error(
                    "Navigation data insert failed: " +
                    navQuery.lastError().text().toStdString());
            }

            currentNavId = navQuery.lastInsertId().toInt();
        }

        // Лямбда для выполнения запросов
        auto executeQuery = [&](QSqlQuery& query, const QString& context) {
            if (!query.exec()) {
                throw std::runtime_error(
                    QString("%1 failed: %2")
                        .arg(context)
                        .arg(query.lastError().text())
                        .toStdString());
            }
        };

        // Обработка данных в зависимости от типа
        switch (data.type) {
        case MsgType::GNRMC: {
            GNRMCData d;
            QDataStream stream(data.data);
            int coordDef, statusNav;
            stream >> d.time >> d.isValid >> d.latitude >> d.longitude
                >> d.speed >> d.course >> d.date >> d.magnDeviation
                >> coordDef >> statusNav;

            d.coordinateDefinition = static_cast<GNRMCData::CoordinateDefinition>(coordDef);
            d.statusNav = static_cast<bool>(statusNav);

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gnrmc_data "
                "(navigation_data_id, time, date, latitude, longitude, "
                "speed, course, isValid, magnDeviation, coordinateDefinition, statusNav) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.time.toString("HH:mm:ss.zzz"));
            q.addBindValue(d.date.toString("yyyy-MM-dd"));
            q.addBindValue(d.latitude);
            q.addBindValue(d.longitude);
            q.addBindValue(d.speed);
            q.addBindValue(d.course);
            q.addBindValue(d.isValid);
            q.addBindValue(d.magnDeviation);
            q.addBindValue(static_cast<int>(d.coordinateDefinition));
            q.addBindValue(d.statusNav);

            executeQuery(q, "GNRMC insert");
            break;
        }

        case MsgType::GNGGA: {
            GNGGAData d;
            QDataStream stream(data.data);
            int coordDef, altUnit, diffUnit;
            stream >> d.time
                >> d.latitude
                >> d.longitude
                >> coordDef
                >> d.satellitesCount
                >> d.HDOP
                >> d.altitude
                >> altUnit
                >> d.diffElipsoidSeaLevel
                >> diffUnit
                >> d.countSecDGPS
                >> d.idDGPS;

            d.coordDef = static_cast<GNGGAData::CoordinateDefinition>(coordDef);
            d.altUnit = static_cast<GNGGAData::AltitudeUnits>(altUnit);
            d.diffElipsUnit = static_cast<GNGGAData::AltitudeUnits>(diffUnit);

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gngga_data "
                "(navigation_data_id, time, latitude, longitude, coordDef, "
                "satellitesCount, hdop, altitude, altUnit, diffElipsoidSeaLevel, "
                "diffElipsUnit, countSecDGPS, idDGPS) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.time.toString("HH:mm:ss.zzz"));
            q.addBindValue(d.latitude);
            q.addBindValue(d.longitude);
            q.addBindValue(static_cast<int>(d.coordDef));
            q.addBindValue(d.satellitesCount);
            q.addBindValue(static_cast<int>(d.HDOP));
            q.addBindValue(d.altitude);
            qDebug()<<d.altitude;
            q.addBindValue(static_cast<int>(d.altUnit));
            q.addBindValue(d.diffElipsoidSeaLevel);
            q.addBindValue(static_cast<int>(d.diffElipsUnit));
            q.addBindValue(static_cast<int>(d.countSecDGPS));
            q.addBindValue(d.idDGPS);

            executeQuery(q, "GNGGA insert");
            break;
        }

        case MsgType::GNGSA: {
            GNGSAData d;
            QDataStream stream(data.data);
            int format, gnss;
            stream >> d.isAuto >> format >> gnss;

            d.typeFormat = static_cast<TypeFormat>(format);
            d.typeGNSS = static_cast<TypeGNSS>(gnss);

            QJsonArray satellites;
            for (int i = 0; i < 12; ++i) {
                stream >> d.satellitesUsedId[i];
                if (d.satellitesUsedId[i] > 0) {
                    satellites.append(d.satellitesUsedId[i]);
                }
            }

            stream >> d.PDOP >> d.HDOP >> d.VDOP;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gngsa_data "
                "(navigation_data_id, isAuto, typeFormat, typeGNSS, "
                "satellitesUsed, pdop, hdop, vdop) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.isAuto);
            q.addBindValue(static_cast<int>(d.typeFormat));
            q.addBindValue(static_cast<int>(d.typeGNSS));
            q.addBindValue(QJsonDocument(satellites).toJson(QJsonDocument::Compact));
            q.addBindValue(static_cast<int>(d.PDOP));
            q.addBindValue(static_cast<int>(d.HDOP));
            q.addBindValue(static_cast<int>(d.VDOP));

            executeQuery(q, "GSA insert");
            break;
        }

        case MsgType::GNZDA: {
            GNZDAData d;
            QDataStream stream(data.data);
            stream >> d.time >> d.date >> d.localOffset;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gnzda_data "
                "(navigation_data_id, time, date, localOffset) "
                "VALUES (?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.time.toString("HH:mm:ss.zzz"));
            q.addBindValue(d.date.toString("yyyy-MM-dd"));
            q.addBindValue(static_cast<int>(d.localOffset));

            executeQuery(q, "GNZDA insert");
            break;
        }

        case MsgType::GNDHV: {
            GNDHVData d;
            QDataStream stream(data.data);
            stream >> d.time >> d.speed3D >> d.speedECEF_X
                >> d.speedECEF_Y >> d.speedECEF_Z >> d.speed;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gndhv_data "
                "(navigation_data_id, time, speed3D, speedECEF_X, "
                "speedECEF_Y, speedECEF_Z, speed) "
                "VALUES (?, ?, ?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.time.toString("HH:mm:ss.zzz"));
            q.addBindValue(d.speed3D);
            q.addBindValue(d.speedECEF_X);
            q.addBindValue(d.speedECEF_Y);
            q.addBindValue(d.speedECEF_Z);
            q.addBindValue(d.speed);

            executeQuery(q, "GNDHV insert");
            break;
        }

        case MsgType::GNGST: {
            GNGSTData d;
            QDataStream stream(data.data);
            stream >> d.time >> d.rms >> d.semiMajorError
                >> d.semiMinorError >> d.semiMajorOrientation
                >> d.latitudeError >> d.longitudeError >> d.altitudeError;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gngst_data "
                "(navigation_data_id, time, rms, semiMajorError, "
                "semiMinorError, semiMajorOrientation, latitudeError, "
                "longitudeError, altitudeError) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.time.toString("HH:mm:ss.zzz"));
            q.addBindValue(d.rms);
            q.addBindValue(d.semiMajorError);
            q.addBindValue(d.semiMinorError);
            q.addBindValue(d.semiMajorOrientation);
            q.addBindValue(d.latitudeError);
            q.addBindValue(d.longitudeError);
            q.addBindValue(d.altitudeError);

            executeQuery(q, "GNGST insert");
            break;
        }

        case MsgType::GPTXT: {
            GPTXTData d;
            QDataStream stream(data.data);
            stream >> d.messageCount >> d.messageNumber >> d.messageType >> d.message;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gptxt_data "
                "(navigation_data_id, messageCount, messageNumber, messageType, message) "
                "VALUES (?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.messageCount);
            q.addBindValue(d.messageNumber);
            q.addBindValue(d.messageType);
            q.addBindValue(d.message);

            executeQuery(q, "GPTXT insert");
            break;
        }

        case MsgType::GNGLL: {
            GNGLLData d;
            QDataStream stream(data.data);
            stream >> d.latitude >> d.longitude >> d.time >> d.isValid;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gngll_data "
                "(navigation_data_id, latitude, longitude, time, isValid) "
                "VALUES (?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.latitude);
            q.addBindValue(d.longitude);
            q.addBindValue(d.time.toString("HH:mm:ss.zzz"));
            q.addBindValue(d.isValid);

            executeQuery(q, "GNGLL insert");
            break;
        }

        case MsgType::GNVTG: {
            GNVTGData d;
            QDataStream stream(data.data);
            stream >> d.trueCourse >> d.magneticCourse
                >> d.speedKnots >> d.speedKmh >> d.isValid;

            QSqlQuery q;
            q.prepare(
                "INSERT INTO gnvtg_data "
                "(navigation_data_id, trueCourse, magneticCourse, "
                "speedKnots, speedKmh, isValid) "
                "VALUES (?, ?, ?, ?, ?, ?)");

            q.addBindValue(currentNavId);
            q.addBindValue(d.trueCourse);
            q.addBindValue(d.magneticCourse);
            q.addBindValue(d.speedKnots);
            q.addBindValue(d.speedKmh);
            q.addBindValue(d.isValid);

            executeQuery(q, "GNVTG insert");
            break;
        }
        default:
            throw std::runtime_error(
                "Unsupported message type: " +
                typeNames.value(data.type, "UNKNOWN").toStdString());
        }

        if (!db.commit()) {
            throw std::runtime_error("Commit failed");
        }

        m_logger->log(Logger::Info, QString("Saved %1 data block in %2 ms")
                                        .arg(typeNames.value(data.type))
                                        .arg(timer.elapsed()));

        return true;
    }
    catch (const std::exception& e) {
        db.rollback();
        logError(QString("Save failed for %1: %2")
                     .arg(typeNames.value(data.type, "UNKNOWN"))
                     .arg(e.what()));
        return false;
    }
}

bool DatabaseManager::insertNewFlight() {
    if (!db.isOpen()) {
        if (m_logger) m_logger->log(Logger::Error, "DB not open for new flight");
        return false;
    }

    firstType = "";

    // Получаем текущее время
    QString currentAt = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    // Генерируем имя рейса
    QString flightName = QString("Flight_%1").arg(currentAt);

    // Вставляем новый рейс в базу данных
    QSqlQuery query;
    query.prepare("INSERT INTO flights (flight_name) "
                  "VALUES (?)");
    query.addBindValue(flightName);

    flight_name = flightName;

    if (!query.exec()) {
        logQueryError("Insert new flight", query);
        return false;
    }

    if (m_logger) {
        m_logger->log(Logger::Info, QString("New flight created: %1 [ID: %2]")
                                        .arg(flightName).arg(query.lastInsertId().toString()));
    }

    navigationDataId=0;
    return true;
}

QString DatabaseManager::getLastFlight() {
    QString flight;

    // Выполняем запрос для получения последнего полета
    QSqlQuery query("SELECT flightName FROM flights ORDER BY id DESC LIMIT 1");

    if (query.exec()) {
        if (query.next()) {
            flight = query.value(0).toString(); // Получаем название последнего полета
        }
    } else {
        logError("Ошибка выполнения запроса: " + query.lastError().text());
    }

    return flight; // Возвращаем название последнего полета
}

QList<QString> DatabaseManager::getAllFlights() {
    QList<QString> flightsList;

    QSqlQuery query("SELECT flight_name FROM flights");
    while (query.next()) {
        // Добавляем только название полета в список
        flightsList.append(query.value(0).toString());
    }

    return flightsList;
}

QList<QVariant> DatabaseManager::getAllFlightsMap() {
    QList<QString> flightsList;

    QSqlQuery query("SELECT flight_name FROM flights");
    while (query.next()) {
        // Добавляем только название полета в список
        flightsList.append(query.value(0).toString());
    }
    QList<QVariant> variantList;
    for (const QString &flight : flightsList) {
        variantList.append(flight); // Убедитесь, что это строка
    }

    return variantList;
}

// Пример метода для получения последнего вставленного ID
int DatabaseManager::getLastInsertedId(const QString &tableName) {
    QSqlQuery query;

    // Выполняем запрос для получения последнего вставленного ID
    if (!query.exec("SELECT last_insert_rowid()")) {
        logError("Ошибка получения последней вставленной строки: " + query.lastError().text());
        return -1; // Возвращаем -1 в случае ошибки
    }

    // Проверяем, есть ли результат
    if (query.next()) {
        return query.value(0).toInt(); // Возвращаем последний вставленный ID
    }

    return -1; // Возвращаем -1, если не удалось получить ID
}

QVector<QPair<QString, QString>> DatabaseManager::getTablesStructure() const {
    return m_tables;
}

QList<NavigationDataTable> DatabaseManager::getCustomData(
    const QStringList& fields,
    const QString& filterField,
    const QString& filterValue,
    const QString& sortField,
    const QString& sortOrder,
    const QString& flightName)
{
    QList<NavigationDataTable> result;

    if (fields.isEmpty() || flightName.isEmpty())
        return result;

    // Маппинг полей сортировки
    QString mappedSortField = mapSortField(sortField);
    QString mappedFilterField = mapFilterField(filterField);

    // Формируем SQL запрос
    QStringList selectFields = {"n.id AS n_id", "n.timestamp AS n_timestamp"};

    // Формируем JOIN-часть
    QString joinClause;
    QHash<QString, QString> tableAliases = {
        {"gnrmc_data", "GNRMC"},
        {"gngga_data", "GNGGA"},
        {"gngsa_data", "GNGSA"},
        {"gnzda_data", "GNZDA"},
        {"gndhv_data", "GNDHV"},
        {"gngst_data", "GNGST"},
        {"gptxt_data", "GPTXT"},
        {"gngll_data", "GNGLL"},
        {"glgsv_data", "GLGSV"},
        {"gnvtg_data", "GNVTG"}
    };

    // Добавляем выбранные поля с алиасами
    for (const QString& field : fields) {
        QStringList parts = field.split(".");
        if (parts.size() != 2) continue;
        QString table = parts[0];
        QString fieldName = parts[1];
        selectFields << QString("%1.%2 AS %3_%2")
                            .arg(tableAliases[table])
                            .arg(fieldName)
                            .arg(table);
    }

    QSet<QString> usedTables;
    for (const QString& field : fields) {
        QString table = field.split(".")[0];
        if (!usedTables.contains(table) && table != "navigation_data") {
            joinClause += QString("LEFT JOIN %1 AS %2 ON n.id = %2.navigation_data_id ")
                              .arg(table)
                              .arg(tableAliases[table]);
            usedTables.insert(table);
        }
    }

    // Формируем полный запрос
    QString queryStr = QString(
                           "SELECT %1 "
                           "FROM navigation_data AS n "
                           "%2 "
                           "WHERE n.flight_name = :flightName "
                           "%3 "  // Фильтр
                           "ORDER BY %4 %5"
                           )
                           .arg(selectFields.join(", "))
                           .arg(joinClause)
                           .arg(!mappedFilterField.isEmpty() && !filterValue.isEmpty() ?
                                    QString("AND %1 LIKE :filterValue").arg(mappedFilterField) : "")
                           .arg(mappedSortField)
                           .arg(sortOrder == "По возрастанию" ? "ASC" : "DESC");

    QSqlQuery query;
    query.prepare(queryStr);
    query.bindValue(":flightName", flightName);

    if (!mappedFilterField.isEmpty() && !filterValue.isEmpty()) {
        query.bindValue(":filterValue", "%" + filterValue + "%");
    }

    if (!query.exec()) {
        logError(QString("Custom query failed: %1\nQuery: %2")
                     .arg(query.lastError().text())
                     .arg(queryStr));
        return result;
    }

    // Обработка результатов
    while (query.next()) {
        NavigationDataTable data;
        data.id = query.value("n_id").toInt();
        data.timestamp = query.value("n_timestamp").toDateTime();

        // Заполняем customData
        for (const QString& field : fields) {
            QStringList parts = field.split(".");
            QString alias = QString("%1_%2").arg(parts[0]).arg(parts[1]);
            data.customData[field] = query.value(alias);
        }

        result.append(data);
    }

    return result;
}

void DatabaseManager::getNavigationDataFilterValidMap(const QString &filterField, const QString &filterValue, const QString &sortField, const QString &sortOrder, const QString &flightName) {
    QList<NavigationData> data = getNavigationDataFilterValid(filterField, filterValue, sortField, sortOrder, flightName);

    QList<NavigationDataMap> mapDataList; // Список для хранения преобразованных данных

    for (const NavigationData &navData : data) {
        NavigationDataMap mapData;

        // Убедитесь, что navData.data - это QByteArray
        QByteArray byteArray = navData.data; // Предполагается, что navData.data - это QByteArray
        QDataStream stream(&byteArray, QIODevice::ReadOnly);

        // Десериализация данных
        stream >> mapData.id
               >> mapData.timestamp
               >> mapData.date
               >> mapData.time
               >> mapData.isValid
               >> mapData.altitude
               >> mapData.latitude
               >> mapData.longitude
               >> mapData.speed
               >> mapData.course;

        // Добавляем в список
        mapDataList.append(mapData);
    }

    // Преобразуем в QVariantList для передачи в QML
    QList<QVariant> variantList;
    for (const NavigationDataMap &mapData : mapDataList) {
        QVariantMap variantMap;
        variantMap["id"] = mapData.id;
        variantMap["date"] = mapData.date;
        variantMap["time"] = mapData.time;
        variantMap["isValid"] = mapData.isValid;
        variantMap["altitude"] = mapData.altitude;
        variantMap["latitude"] = mapData.latitude;
        variantMap["longitude"] = mapData.longitude;
        variantMap["speed"] = mapData.speed;
        variantMap["course"] = mapData.course;

        variantList.append(variantMap);
    }

    emit dataLoaded(variantList); // Передаем преобразованные данные в сигнал
}

QString DatabaseManager::mapSortField(const QString &uiField) {
    static const QMap<QString, QString> fieldMap = {
        {"ID",         "n_id"},
        {"latitude",   "gnrmc_latitude"},
        {"longitude",  "gnrmc_longitude"},
        {"speed",      "gnrmc_speed"},
        {"course",     "gnrmc_course"},
        {"isValid",    "gnrmc_isValid"},
        {"altitude",   "gngga_altitude"},
        {"time",       "gnzda_time"},
        {"date",       "gnzda_date"},
        {"timestamp",  "n_timestamp"}
    };

    // Значение по умолчанию, если поле не найдено
    static const QString defaultField = "n_id"; // Например, сортировка по времени

    return fieldMap.value(uiField, defaultField);
}

QString DatabaseManager::mapFilterField(const QString &uiField) {
    static const QMap<QString, QString> fieldMap = {
        {"ID",         "n_id"},
        {"Latitude",   "gnrmc_latitude"},
        {"Longitude",  "gnrmc_longitude"},
        {"Speed",      "gnrmc_speed"},
        {"Course",     "gnrmc_course"},
        {"IsValid",    "gnrmc_isValid"},
        {"Altitude",   "gngga_altitude"},
        {"Time",       "gnzda_time"},
        {"Date",       "gnzda_date"},
        {"TimeStamp",  "n_timestamp"}
    };

    // Значение по умолчанию, если поле не найдено
    static const QString defaultField = ""; // Например, сортировка по времени

    return fieldMap.value(uiField, defaultField);
}

QList<NavigationData> DatabaseManager::getNavigationDataFilter(QString &filterField,
                                                               QString &filterValue,
                                                               const QString &sortField,
                                                               const QString &sortOrder,
                                                               const QString &flightName)
{
    QList<NavigationData> navigationDataList;

    // Логирование входных параметров
    m_logger->log(Logger::Debug,
                  QString("Запрос данных. Параметры:\n"
                          " - Поле фильтра: %1\n"
                          " - Значение фильтра: %2\n"
                          " - Поле сортировки: %3\n"
                          " - Направление сортировки: %4\n"
                          " - Полет: %5")
                      .arg(filterField)
                      .arg(filterValue)
                      .arg(sortField)
                      .arg(sortOrder)
                      .arg(flightName));

    // Маппинг полей
    QString dbSortField = mapSortField(sortField);
    QString dbFilterField = mapFilterField(filterField);

    // Сброс фильтра при пустом значении
    if (filterValue.isEmpty()) {
        m_logger->log(Logger::Debug, "Значение фильтра пусто. Фильтрация отключена.");
        dbFilterField = "";
        filterValue = "";
    }

    // Формирование SQL-запроса
    QString filterClause = "";
    if (!dbFilterField.isEmpty() && !filterValue.isEmpty()) {
        filterClause = QString(" AND %1 = :filter_value").arg(dbFilterField);
    }

    // Формируем SQL-запрос с параметрами
    QString queryStr = QString(
                           "SELECT gnrmc.id AS gnrmc_id, gnrmc.time AS gnrmc_time, gnrmc.date AS gnrmc_date, "
                           "gnrmc.latitude AS gnrmc_latitude, gnrmc.longitude AS gnrmc_longitude, "
                           "gnrmc.speed AS gnrmc_speed, gnrmc.course AS gnrmc_course, gnrmc.isValid AS gnrmc_isValid, "
                           "gngga.altitude AS gngga_altitude, "
                           "gnzda.time AS gnzda_time, gnzda.date AS gnzda_date, "
                           "n.id AS n_id, n.timestamp AS n_timestamp "
                           "FROM navigation_data n "
                           "LEFT JOIN gnrmc_data gnrmc ON gnrmc.navigation_data_id = n.id "
                           "LEFT JOIN gnzda_data gnzda ON gnzda.navigation_data_id = n.id "
                           "LEFT JOIN gngga_data gngga ON gngga.navigation_data_id = n.id "
                           "WHERE n.flight_name = :flight_name%1 "
                           "ORDER BY %2 %3")
                           .arg(filterClause)
                           .arg(dbSortField)
                           .arg(sortOrder == "DESC" ? "DESC" : "ASC");

    QSqlQuery query;
    query.prepare(queryStr);

    // Привязка параметров
    query.bindValue(":flight_name", flightName);

    if (!filterField.isEmpty() && !filterValue.isEmpty()) {
        query.bindValue(":filter_value", filterValue);
    }

    // Выполнение запроса
    if (!query.exec()) {
        m_logger->log(Logger::Error,
                      QString("Ошибка выполнения запроса!\n"
                              " - Текст ошибки: %1\n")
                          .arg(query.lastError().text()));
        return navigationDataList;
    }

    // Обработка результатов
    while (query.next()) {
        NavigationData data;
        data.id = query.value("n_id").toInt();
        data.timestamp = QDateTime::fromString(query.value("n_timestamp").toString(), "yyyy-MM-dd hh:mm:ss");

        GNRMCData gnrmc;
        gnrmc.isValid = query.value("gnrmc_isValid").toInt() ? 1 : 0;
        gnrmc.latitude = query.value("gnrmc_latitude").toDouble();
        gnrmc.longitude = query.value("gnrmc_longitude").toDouble();
        gnrmc.speed = query.value("gnrmc_speed").toDouble();
        gnrmc.course = query.value("gnrmc_course").toDouble();

        GNGGAData gngga;
        gngga.altitude = query.value("gngga_altitude").toFloat();

        GNZDAData gnzda;
        gnzda.time = query.value("gnzda_time").toTime();
        gnzda.date = query.value("gnzda_date").toDate();

        QByteArray byteArray;
        QDataStream stream(&byteArray, QIODevice::WriteOnly);
        stream << data.id
               << data.timestamp
               << gnzda.date
               << gnzda.time
               << gnrmc.isValid
               << gngga.altitude
               << gnrmc.latitude
               << gnrmc.longitude
               << gnrmc.speed
               << gnrmc.course;
        data.data = byteArray;

        navigationDataList.append(data);
    }

    qDebug()<<navigationDataList.count();

    return navigationDataList;
}

QList<NavigationData> DatabaseManager::getNavigationDataFilterValid(const QString &filterField,
                                                                    const QString &filterValue,
                                                                    const QString &sortField,
                                                                    const QString &sortOrder,
                                                                    const QString &flightName)
{
    QList<NavigationData> navigationDataList;

    // Логирование входных параметров
    m_logger->log(Logger::Debug,
                  QString("Запрос данных (валидные). Параметры:\n"
                          " - Поле фильтра: %1\n"
                          " - Значение фильтра: %2\n"
                          " - Поле сортировки: %3\n"
                          " - Направление сортировки: %4\n"
                          " - Полет: %5")
                      .arg(filterField)
                      .arg(filterValue)
                      .arg(sortField)
                      .arg(sortOrder)
                      .arg(flightName));

    // Маппинг полей
    QString dbSortField = mapSortField(sortField);
    QString dbFilterField = mapFilterField(filterField);

    // Формирование фильтра
    QString filterClause = "gnrmc.isValid = 1"; // Основное условие для валидных данных
    if (!dbFilterField.isEmpty() && !filterValue.isEmpty()) {
        filterClause += QString(" AND %1 = :filter_value").arg(dbFilterField);
    }

    // Формируем SQL-запрос
    QString queryStr = QString(
                           "SELECT gnrmc.id AS gnrmc_id, gnrmc.time AS gnrmc_time, gnrmc.date AS gnrmc_date, "
                           "gnrmc.latitude AS gnrmc_latitude, gnrmc.longitude AS gnrmc_longitude, "
                           "gnrmc.speed AS gnrmc_speed, gnrmc.course AS gnrmc_course, gnrmc.isValid AS gnrmc_isValid, "
                           "gngga.altitude AS gngga_altitude, "
                           "gnzda.time AS gnzda_time, gnzda.date AS gnzda_date, "
                           "n.id AS n_id, n.timestamp AS n_timestamp "
                           "FROM navigation_data n "
                           "LEFT JOIN gnrmc_data gnrmc ON gnrmc.navigation_data_id = n.id "
                           "LEFT JOIN gnzda_data gnzda ON gnzda.navigation_data_id = n.id "
                           "LEFT JOIN gngga_data gngga ON gngga.navigation_data_id = n.id "
                           "WHERE n.flight_name = :flight_name AND %1 "  // Добавляем условие валидности
                           "ORDER BY %2 %3")
                           .arg(filterClause)
                           .arg(dbSortField)
                           .arg(sortOrder == "DESC" ? "DESC" : "ASC");

    QSqlQuery query;
    query.prepare(queryStr);

    // Привязка параметров
    query.bindValue(":flight_name", flightName);
    if (!dbFilterField.isEmpty() && !filterValue.isEmpty()) {
        query.bindValue(":filter_value", filterValue);
    }

    // Выполнение запроса
    if (!query.exec()) {
        m_logger->log(Logger::Error,
                      QString("Ошибка выполнения запроса!\n"
                              " - Текст ошибки: %1\n")
                          .arg(query.lastError().text()));
        return navigationDataList;
    }

    // Обработка результатов
    while (query.next()) {
        NavigationData data;
        data.id = query.value("n_id").toInt();
        data.timestamp = QDateTime::fromString(query.value("n_timestamp").toString(), Qt::ISODateWithMs);

        GNRMCData gnrmc;
        gnrmc.isValid = query.value("gnrmc_isValid").toBool();
        gnrmc.latitude = query.value("gnrmc_latitude").toDouble();
        gnrmc.longitude = query.value("gnrmc_longitude").toDouble();
        gnrmc.speed = query.value("gnrmc_speed").toDouble();
        gnrmc.course = query.value("gnrmc_course").toDouble();

        GNGGAData gngga;
        gngga.altitude = query.value("gngga_altitude").toDouble();

        GNZDAData gnzda;
        gnzda.time = query.value("gnzda_time").toTime();
        gnzda.date = query.value("gnzda_date").toDate();

        // Сериализация данных
        QByteArray byteArray;
        QDataStream stream(&byteArray, QIODevice::WriteOnly);
        stream << data.id
               << data.timestamp
               << gnzda.date
               << gnzda.time
               << gnrmc.isValid
               << gngga.altitude
               << gnrmc.latitude
               << gnrmc.longitude
               << gnrmc.speed
               << gnrmc.course;
        data.data = byteArray;

        navigationDataList.append(data);
    }

    m_logger->log(Logger::Debug, QString("Найдено записей: %1").arg(navigationDataList.count()));
    return navigationDataList;
}

// Метод для удаления навигационных данных по ID
bool DatabaseManager::deleteNavigationDataById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM navigation_data WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        logError("Ошибка удаления данных из базы: " + query.lastError().text());
        return false;
    }
    return true;
}

QList<QVariant> DatabaseManager::getNavigationDataMap() {
    QList<QVariant> navigationDataList;

    if (!db.isOpen()) {
        return navigationDataList;
    }

    QSqlQuery query("SELECT * FROM navigation_data");
    while (query.next()) {
        QVariantMap dataMap;

        // Заполнение QVariantMap данными из запроса
        dataMap["flyName"] = query.value("flyName").toString();
        dataMap["gngga_id"] = query.value("gngga_id").toInt();
        dataMap["gnrmc_id"] = query.value("gnrmc_id").toInt();
        dataMap["gnzda_id"] = query.value("gnzda_id").toInt();
        dataMap["gndhv_id"] = query.value("gndhv_id").toInt();
        dataMap["gngst_id"] = query.value("gngst_id").toInt();
        dataMap["gnvtg_id"] = query.value("gnvtg_id").toInt();
        // Добавьте другие поля по мере необходимости

        navigationDataList.append(dataMap); // Добавляем QVariantMap в список
    }

    return navigationDataList;
}

QByteArray DatabaseManager::getCompleteFlightData(const QString &flightName) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // Список всех связанных таблиц
    const QMap<QString, QString> tables = {
        {"gnrmc_data", "GNRMC"},
        {"gngga_data", "GNGGA"},
        {"gngsa_data", "GNGSA"},
        {"gnzda_data", "GNZDA"},
        {"gndhv_data", "GNDHV"},
        {"gngst_data", "GNGST"},
        {"gptxt_data", "GPTXT"},
        {"gngll_data", "GNGLL"},
        {"glgsv_data", "GLGSV"},
        {"gnvtg_data", "GNVTG"}
    };

    // Основные данные навигации
    QSqlQuery navQuery;
    navQuery.prepare("SELECT * FROM navigation_data WHERE flight_name = ?");
    navQuery.addBindValue(flightName);

    if(navQuery.exec()) {
        while(navQuery.next()) {
            QVariantMap navData;
            for(int i = 0; i < navQuery.record().count(); ++i) {
                QString field = navQuery.record().fieldName(i);
                navData[field] = navQuery.value(i);
            }
            stream << "navigation_data" << navData;
        }
    }

    // Данные из всех дочерних таблиц
    for(auto it = tables.constBegin(); it != tables.constEnd(); ++it) {
        QSqlQuery query;
        query.prepare(QString("SELECT * FROM %1 WHERE navigation_data_id IN "
                              "(SELECT id FROM navigation_data WHERE flight_name = ?)").arg(it.key()));
        query.addBindValue(flightName);

        if(query.exec()) {
            while(query.next()) {
                QVariantMap record;
                for(int i = 0; i < query.record().count(); ++i) {
                    QString field = query.record().fieldName(i);
                    record[field] = query.value(i);
                }
                stream << it.value() << record;
            }
        }
    }

    return data;
}

QString DatabaseManager::getRawFlightData(const QString &flightName) {
    QJsonArray data;

    // Получение данных из всех связанных таблиц
    const QStringList tables = {
        "navigation_data", "gnrmc_data", "gngga_data", "gngsa_data",
        "gnzda_data", "gndhv_data", "gngst_data", "gptxt_data",
        "gngll_data", "glgsv_data", "gnvtg_data"
    };

    QSqlQuery query;
    foreach (const QString &table, tables) {
        query.prepare(QString("SELECT * FROM %1 WHERE navigation_data_id IN "
                              "(SELECT id FROM navigation_data WHERE flight_name = ?)").arg(table));
        query.addBindValue(flightName);

        if (!query.exec()) {
            qWarning() << "Ошибка выполнения запроса для таблицы" << table << ":" << query.lastError();
            continue;
        }

        while (query.next()) {
            QJsonObject record;
            for(int i = 0; i < query.record().count(); ++i) {
                record[query.record().fieldName(i)] = QJsonValue::fromVariant(query.value(i));
            }
            data.append(QJsonObject{
                {"table", table},
                {"data", record}
            });
        }
    }

    return QJsonDocument(data).toJson();
}

QString DatabaseManager::getFlightDataAsJson(const QString &flightName) {
    QJsonArray data;

    // Пример получения данных из таблицы gnrmc_data
    QSqlQuery query;
    query.prepare("SELECT * FROM gnrmc_data WHERE navigation_data_id IN "
                  "(SELECT id FROM navigation_data WHERE flight_name = ?)");
    query.addBindValue(flightName);

    if(query.exec()) {
        while(query.next()) {
            QJsonObject record;
            for(int i = 0; i < query.record().count(); ++i) {
                record[query.record().fieldName(i)] = QJsonValue::fromVariant(query.value(i));
            }
            data.append(record);
        }
    }

    return QJsonDocument(data).toJson();
}

// Метод для логирования ошибок
void DatabaseManager::logError(const QString &message) {
    m_logger->log(Logger::Error, message);
}
