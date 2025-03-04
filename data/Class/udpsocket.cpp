#include "udpsocket.h"

UdpSocket::UdpSocket(QObject *parent) : QUdpSocket(parent) {}

void UdpSocket::sendData(const QByteArray &data, const QHostAddress &address, quint16 port) {
    writeDatagram(data, address, port); // Use writeDatagram instead of sendTo
}
