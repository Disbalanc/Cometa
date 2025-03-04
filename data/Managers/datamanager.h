#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "databasemanager.h"
#include "formatnavigationdata.h"
#include "logger.h"
#include "parsernmea.h"

#include <QObject>
#include <QFile>
#include <QDir>
#include <QThread>

class DataManager : public QObject {
    Q_OBJECT

public:
    explicit DataManager(DatabaseManager *dbManager, QObject *parent);

    void processLogFile(const QString &filePath);
    void cancelProcessing();

    void saveNavigationData(const NavigationData &navData);
    void writeDataToFile(const QByteArray &data);
    void saveFile(const QString &flightName);
    void setLogger(Logger *logger);

signals:
    void dataProcessed(int processedCount, int savedCount,std::chrono::duration<double> duration);
    void errorOccurred(const QString &error);

private:
    QFile* currentFile = nullptr;
    qint64 filePosition = 0;
    int processedCount = 0;
    int savedCount = 0;
    double minLatitude;
    double maxLatitude;
    double minLongtitude;
    double maxLongtitude;
    double minAltitude;
    double maxAltitude;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    bool isProcessing = false;

    void processNextBlock();
    void finishProcessing();
    void showSummaryMessage(const QString& summary);

    QThread m_workerThread;
    QString filePath;
    NavigationDataFormatter *formatNavigation;
    DatabaseManager *dbManager;
    ParserNMEA parser;
    Logger *m_logger;
};

#endif // DATAMANAGER_H
