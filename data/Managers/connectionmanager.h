#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QComboBox>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "databasemanager.h"
#include "datamanager.h"
#include "ethernetclient.h"
#include "formatnavigationdata.h"
#include "logger.h"
#include "parsernmea.h"

class ConnectionManager : public QObject {
    Q_OBJECT

public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager();

    void setLogger(Logger *logger);

    void connectToTTL(const QString &portName);
    void connectToEthernet(const QString &ipAddress, quint16 port);
    void disconnect();
    void populateSerialPorts(QComboBox *serialPortComboBox); // Передаем QComboBox для заполнения
    void onEthernetDataReceived(const QByteArray &receivedData);
    void onReadyRead();
    void processReceivedData(const QByteArray &data);
    void configureSerialPort();
    QSerialPort* getSerialPort(); // Метод для получения указателя на QSerialPort

signals:
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
    void dataFormatted(const QString &formattedData);

private:
    NavigationDataFormatter m_formatter; // Добавляем форматтер
    Logger *m_logger;
    QSerialPort *serialPort;
    EthernetClient *ethernetClient;
    DatabaseManager *dbManager;
    DataManager *dataManager;
    ParserNMEA parser;
};

#endif // CONNECTIONMANAGER_H
