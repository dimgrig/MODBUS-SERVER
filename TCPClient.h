#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QAbstractSocket>
#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWidget>

#include "Config.h"

#include "Logger.h"
#include "Sleeper.h"

class TCPClient : public QWidget {
Q_OBJECT
public:
    TCPClient(QWidget *parent = nullptr);
    virtual ~TCPClient();

    bool isConnected() {return this->_socket->isOpen();};
signals:
//    void port_error(QString error);
//    void port_opened();
//    void port_closed();
    void read_data(QByteArray rxBuff);
public slots:
//    void openSerialPort(QString port, int speed);
//    void closeSerialPort();
    void writeData(const QByteArray &txBuff);
private slots:
#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
    void handleError(QAbstractSocket::SocketError error);
#endif
//    void on_timer();
private:
    void readData();

private:
    QTcpSocket* _socket = new QTcpSocket(this);
};

#endif // TCPCLIENT_H
