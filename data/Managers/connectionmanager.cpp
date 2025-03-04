#include "connectionmanager.h"

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent),
    serialPort(new QSerialPort(this)),
    ethernetClient(nullptr)
{

}

ConnectionManager::~ConnectionManager() {
    // Освобождаем ресурсы
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
    delete serialPort;

    if (ethernetClient) {
        ethernetClient->disconnectFromServer();
        delete ethernetClient;
    }
}

void ConnectionManager::setLogger(Logger *logger) {
    m_logger = logger;
}

void ConnectionManager::connectToTTL(const QString &portName)
{
    try {
        m_logger->log(Logger::Debug, QString("Attempting TTL connection to %1").arg(portName));
        serialPort->setPortName(portName);

        if (serialPort->open(QIODevice::ReadOnly)) {
            configureSerialPort();

            connect(serialPort, &QSerialPort::readyRead, this, &ConnectionManager::onReadyRead);
            m_logger->log(Logger::Info, QString("Successfully connected to %1").arg(portName));
            emit connectionStatusChanged(true);
        } else {
            QString error = QString("Failed to open serial port: %1").arg(serialPort->errorString());
            m_logger->log(Logger::Error, error);
            emit errorOccurred(error);
        }
    } catch (const std::exception &e) {
        m_logger->log(Logger::Critical, QString("TTL connection error: %1").arg(e.what()));
        throw;
    }
}

void ConnectionManager::populateSerialPorts(QComboBox *serialPortComboBox) {
    serialPortComboBox->clear();
    foreach (const QSerialPortInfo &port, QSerialPortInfo::availablePorts()) {
        serialPortComboBox->addItem(port.portName());
    }
}

void ConnectionManager::configureSerialPort() {
    // Настройки порта по умолчанию
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
}

void ConnectionManager::connectToEthernet(const QString &ipAddress, quint16 port)
{
    try {
        m_logger->log(Logger::Debug, QString("Attempting Ethernet connection to %1:%2").arg(ipAddress).arg(port));
        ethernetClient = new EthernetClient(ipAddress, port, this);
        connect(ethernetClient, &EthernetClient::errorOccurred, this, [this](const QString &error) {
            m_logger->log(Logger::Error, QString("Ethernet error: %1").arg(error));
            emit errorOccurred(error);
        });

        if (ethernetClient->connectToServer()) {
            connect(ethernetClient, &EthernetClient::dataReceived, this, &ConnectionManager::onEthernetDataReceived);
            m_logger->log(Logger::Info, "Ethernet connection established");
            emit connectionStatusChanged(true);
        } else {
            QString error = "Failed to connect via Ethernet";
            m_logger->log(Logger::Error, error);
            emit errorOccurred(error);
        }
    } catch (const std::exception &e) {
        m_logger->log(Logger::Critical, QString("Ethernet connection error: %1").arg(e.what()));
        throw;
    }
}

void ConnectionManager::disconnect()
{
    m_logger->log(Logger::Info, "Disconnecting...");

    if (serialPort->isOpen()) {
        serialPort->close();
        m_logger->log(Logger::Debug, "Serial port closed");
        emit connectionStatusChanged(false);
    }

    if (ethernetClient) {
        ethernetClient->disconnectFromServer();
        m_logger->log(Logger::Debug, "Ethernet connection closed");
        emit connectionStatusChanged(false);
    }
}

void ConnectionManager::onEthernetDataReceived(const QByteArray &receivedData)
{
    m_logger->log(Logger::Debug, QString("Received %1 bytes via Ethernet").arg(receivedData.size()));
    dataManager->writeDataToFile(receivedData);
    processReceivedData(receivedData);
}

void ConnectionManager::onReadyRead()
{
    QByteArray data = serialPort->readAll();
    m_logger->log(Logger::Debug, QString("Received %1 bytes via TTL").arg(data.size()));
    dataManager->writeDataToFile(data);
    processReceivedData(data);
}

void ConnectionManager::processReceivedData(const QByteArray &data)
{
    try {
        QStringList lines = QString::fromUtf8(data).split('\n', QString::SkipEmptyParts);
        m_logger->log(Logger::Debug, QString("Processing %1 data lines").arg(lines.count()));

        int validCount = 0;
        int invalidCount = 0;

        for (const QString &line : lines) {
            QString cleanedLine = line.trimmed();
            if (cleanedLine.isEmpty() || !cleanedLine.startsWith('$')) {
                m_logger->log(Logger::Warning, QString("Invalid data line: %1").arg(cleanedLine.left(50)));
                invalidCount++;
                continue;
            }

            NavigationData parsedData = parser.parseData(cleanedLine);
            if (parsedData.result == OK) {
                // Форматируем данные
                QString formatted = m_formatter.formatNavigationData(parsedData);

                // Логируем отформатированные данные
                m_logger->log(Logger::Info, "Parsed data:\n" + formatted);

                // Отправляем в интерфейс (если нужно)
                emit dataFormatted(formatted);

                dataManager->saveNavigationData(parsedData);
                validCount++;
            } else {
                m_logger->log(Logger::Warning, QString("Failed to parse: %1").arg(cleanedLine.left(50)));
                invalidCount++;
            }
        }

        m_logger->log(Logger::Info, QString("Data processing complete. Valid: %1, Invalid: %2").arg(validCount).arg(invalidCount));
    } catch (const std::exception &e) {
        m_logger->log(Logger::Error, QString("Data processing error: %1").arg(e.what()));
        emit errorOccurred(e.what());
    }
}
