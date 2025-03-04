#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "logger.h"
#include "parsernmea.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QList>
#include <QDebug>
#include <QSqlRecord>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaType>

Q_DECLARE_METATYPE(NavigationDataMap)

class DatabaseManager: public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(const QString &dbName, QObject *parent = nullptr);
    DatabaseManager(QObject *parent = nullptr) : QObject(parent) {
        // Инициализация базы данных с некоторым значением по умолчанию
        db = QSqlDatabase::addDatabase("QSQLITE");
        //db.setDatabaseName("default.db"); // Установите имя базы данных по умолчанию
        initializeDatabase();
    }
    void setLogger(Logger *logger);

    // В DatabaseManager добавить:
    QVector<QPair<QString, QString>> getTablesStructure() const;

    QList<NavigationDataTable> getCustomData(
        const QStringList& fields,
        const QString& filterField,
        const QString& filterValue,
        const QString& sortField,
        const QString& sortOrder,
        const QString& flightName
        );

    QByteArray getCompleteFlightData(const QString &flightName);
    QString getRawFlightData(const QString &flightName);
    QString getFlightDataAsJson(const QString &flightName);
    Q_INVOKABLE bool open();
    void close();
    bool createTables(const QVector<QPair<QString, QString>>& tables);
    bool initializeDatabase();
    int getLastInsertedId(const QString &tableName);
    int navigationDataId;

    bool saveNavigationData(const NavigationData &data);

    bool deleteFlight(const QString &flightName);
    bool deleteNavigationDataById(int id);
    bool insertNewFlight();

    Q_INVOKABLE QList<QString> getAllFlights();
    QString getLastFlight();
    Q_INVOKABLE QList<QVariant> getAllFlightsMap();
    Q_INVOKABLE QList<QVariant> getNavigationDataMap();
    QList<NavigationData> getNavigationDataFilter(QString &filterField,
                                                QString &filterValue,
                                                const QString &sortField,
                                                const QString &sortOrder,
                                                const QString &flightName);
    QList<NavigationData> getNavigationDataFilterValid(const QString &filterField,
                                                       const QString &filterValue,
                                                       const QString &sortField,
                                                       const QString &sortOrder,
                                                       const QString &flightName);
    QString mapSortField(const QString &uiField);
    QString mapFilterField(const QString &uiField);
    NavigationData getLatestNavigationData();
    void validateTableStructure(const QString &tableName);
    void logQueryDetails(const QSqlQuery &query);
    bool executeTimedQuery(QSqlQuery &query);
    void logQueryStats();
    void logQueryError(const QString &operation, const QSqlQuery &query);
signals:
    void databaseOpened();
    void databaseError(const QString &error);
    void dataLoaded(const QList<QVariant> &data);

public slots:
    void getNavigationDataFilterValidMap(const QString &filterField,
                                         const QString &filterValue,
                                         const QString &sortField,
                                         const QString &sortOrder,
                                         const QString &flightName);

private:
    QVector<QPair<QString, QString>> m_tables;
    struct {
        int totalQueries = 0;
        int failedQueries = 0;
        QMap<QString, int> queryCounts;
    } stats;

    int currentNavId;
    QString flight_name;
    QString firstType;
    ParserNMEA parser;
    Logger *m_logger;
    void logError(const QString &message);
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
