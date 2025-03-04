#include "ethernetclient.h"
#include <QDebug>
#include <QCoreApplication>

EthernetClient::EthernetClient(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), socket(new UdpSocket(this)), host(host), port(port) {

    // Привязка сокета к конкретному хосту и порту
    QHostAddress address = QHostAddress::Any;
    if (!socket->bind(address, port)) {
        qDebug() << "Ошибка привязки сокета:" << socket->errorString();
        emit errorOccurred("Не удалось привязать сокет к адресу " + host + " и порту " + QString::number(port) + ": " + socket->errorString());
    } else {
        qDebug() << "Сокет успешно привязан к адресу" << host << "и порту" << port;
    }

    connect(socket, &UdpSocket::readyRead, this, &EthernetClient::onReadyRead);

    retryTimer = new QTimer(this);
    connect(retryTimer, &QTimer::timeout, this, &EthernetClient::retryConnection);
}

bool EthernetClient::connectToServer() {
    // Отправляем команду подключения к устройству
    QByteArray connectCommand = "CONNECT";
    QHostAddress address(host);
    socket->writeDatagram(connectCommand, address, port);

    // Ожидаем ответа от устройства
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(1000); // 1 секунда ожидания

    while (timer.isActive()) {
        if (socket->hasPendingDatagrams()) {
            QByteArray response;
            response.resize(int(socket->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;

            socket->readDatagram(response.data(), response.size(), &sender, &senderPort);
            if (response == "CONNECTED") {
                return true; // Устройство подключено
            }
        }
        QCoreApplication::processEvents(); // Обрабатываем события, чтобы не блокировать интерфейс
    }

    return false; // Устройство не подключено
}

void EthernetClient::disconnectFromServer() {
    if (socket->isOpen()) {
        socket->close(); // Закрываем сокет
        qDebug() << "Соединение с сервером закрыто.";
    } else {
        qDebug() << "Сервер уже отключен.";
    }
}

void EthernetClient::sendData(const QByteArray &data) {
    if (pingDevice()) {
        QHostAddress address(host);
        socket->writeDatagram(data, address, port);
        qDebug() << "Data sent:" << data;
    } else {
        emit errorOccurred("Устройство недоступно.");
    }
}

void EthernetClient::onReadyRead() {
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(socket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        emit dataReceived(datagram);
        qDebug() << "Data received:" << datagram; // Отладочное сообщение
    }
}

void EthernetClient::handleError(QAbstractSocket::SocketError socketError) {
    emit errorOccurred("Socket error: " + socket->errorString());
    qDebug() << "Socket error:" << socket->errorString();
}

void EthernetClient::retryConnection() {
    if (!connectToServer()) {
        qDebug() << "Retrying connection...";
    }
}

bool EthernetClient::pingDevice() {
    QByteArray pingData = "PING"; // Специальный пакет для пинга
    QHostAddress address(host);

    // Отправляем пинг
    socket->writeDatagram(pingData, address, port);

    // Устанавливаем таймер ожидания
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(1000); // 1 секунда ожидания

    // Ожидаем ответа
    while (timer.isActive()) {
        if (socket->hasPendingDatagrams()) {
            QByteArray response;
            response.resize(int(socket->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;

            socket->readDatagram(response.data(), response.size(), &sender, &senderPort);
            if (response == pingData) {
                return true; // Устройство доступно
            }
        }
        QCoreApplication::processEvents(); // Обрабатываем события, чтобы не блокировать интерфейс
    }

    return false; // Устройство недоступно
}
