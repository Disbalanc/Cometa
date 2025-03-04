#ifndef ETHERNETCLIENT_H
#define ETHERNETCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>

#include "udpsocket.h"

class EthernetClient : public QObject {
    Q_OBJECT

public:
    explicit EthernetClient(const QString &host, quint16 port, QObject *parent = nullptr);
    bool connectToServer();
    void disconnectFromServer();
    void sendData(const QByteArray &data);

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);
    void retryConnection();

private:
    UdpSocket *socket;
    QString host;
    quint16 port;
    QTimer *retryTimer;

    bool pingDevice();
};

#endif // ETHERNETCLIENT_H
