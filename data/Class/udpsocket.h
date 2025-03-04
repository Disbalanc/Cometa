#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <QUdpSocket>
#include <QHostAddress>

class UdpSocket : public QUdpSocket {
    Q_OBJECT
public:
    explicit UdpSocket(QObject *parent = nullptr);
    void sendData(const QByteArray &data, const QHostAddress &address, quint16 port);
};

#endif // UDPSOCKET_H
