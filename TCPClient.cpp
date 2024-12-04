#include "TCPClient.h"

TCPClient::TCPClient(QWidget *parent) :
        QWidget(parent)
{
    //_socket->setSocketOption(QAbstractSocket::KeepAliveOption,1);
    //_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

//    connect(_socket,SIGNAL(readyRead()),SLOT(readSocketData()),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(error(QAbstractSocket::SocketError)),SIGNAL(connectionError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),SIGNAL(tcpSocketState(QAbstractSocket::SocketState)),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(disconnected()),SLOT(onConnectionTerminated()),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(connected()),SLOT(onConnectionEstablished()),Qt::UniqueConnection);

    connect(_socket, &QTcpSocket::readyRead, this, &TCPClient::readData);
#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
    connect(_socket, &QTcpSocket::errorOccurred, this, &TCPClient::handleError);
#endif
    connect(_socket, &QTcpSocket::connected, this, [=](){
        qCDebug(logInfo()) << "TCP: connected to" << cfg.IP << cfg.PORT;
    });
    connect(_socket, &QTcpSocket::disconnected, this, [=](){
        qCDebug(logInfo()) << "TCP: disconnected from" << cfg.IP << cfg.PORT;
    });

    if(!(QAbstractSocket::ConnectedState == _socket->state())){
        _socket->connectToHost(cfg.IP, cfg.PORT, QIODevice::ReadWrite);
    }   

//    int enableKeepAlive = 1;
//    int fd = _socket->socketDescriptor();
//    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));

//    int maxIdle = 60; /* seconds */
//    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));

//    int count = 3;  // send up to 3 keepalive packets out, then disconnect if no response
//    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));

//    int interval = 2;   // send a keepalive packet out every 2 seconds (after the 5 second idle period)
//    setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
}

TCPClient::~TCPClient()
{

}

#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
void TCPClient::handleError(QAbstractSocket::SocketError socketError){
    _socket->disconnectFromHost();
    if (_socket->state() == QAbstractSocket::UnconnectedState || _socket->waitForDisconnected(1000)) {

    }

    switch(socketError){
        case QAbstractSocket::RemoteHostClosedError:
            qCDebug(logWarning()) << "TCP: RemoteHostClosedError";
        break;
        case QAbstractSocket::HostNotFoundError:
            qCDebug(logWarning()) << "TCP: HostNotFoundError. Connection was refused. Server is not working";
        break;
        case QAbstractSocket::ConnectionRefusedError:
            qCDebug(logWarning()) << "TCP: ConnectionRefusedError. Client. The host was not found";
        break;
        default:
            qCDebug(logWarning()) << "TCP: Error" << _socket->errorString();
        break;
    }

    if(!(QAbstractSocket::ConnectedState == _socket->state())){
        //_socket->connectToHost(cfg.IP, cfg.PORT, QIODevice::ReadWrite);
        QTimer::singleShot(0, this, [=](){
            _socket->connectToHost(cfg.IP, cfg.PORT, QIODevice::ReadWrite);
        });
    }
    //_socket->reset();
}
#endif

void TCPClient::writeData(const QByteArray &txBuff){
//    QByteArray l_vDataToBeSent;
//    QDataStream l_vStream(&l_vDataToBeSent, QIODevice::WriteOnly);
//    l_vStream.setByteOrder(QDataStream::LittleEndian);
//    l_vStream << txBuff.length();
//    l_vDataToBeSent.append(txBuff);

//    _socket->write(l_vDataToBeSent, l_vDataToBeSent.length());
//    _socket->flush();

//    if (cfg.DEBUG_MSG_UART) {
//        qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
//    }
    _socket->write(txBuff.data(), txBuff.length());
    _socket->flush();

    if (cfg.DEBUG_MSG_UART) {
        qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
    }
//    if (_socket) {
//        if (_socket->isOpen()) {
//            QDataStream socketStream(_socket);
//            //socketStream.setVersion(QDataStream::Qt_5_15);
//            socketStream << txBuff;
//            qCDebug(logDebug()) << Qt::hex << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
//        } else {
//            qCDebug(logWarning()) << "TCP: Socket doesn't seem to be opened";
//        }
//    } else {
//        qCDebug(logWarning()) << "TCP: Not connected";
//    }
}

void TCPClient::readData(){
    QByteArray rxBuff;
    while(_socket->bytesAvailable()){
        rxBuff += _socket->readAll();

        if (cfg.DEBUG_MSG_UART) {
            qCDebug(logDebug()) << QT_HEX << "TCP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
        }
    }

    emit read_data(rxBuff);
//    QByteArray rxBuff;

//    QDataStream socketStream(_socket);
//    //socketStream.setVersion(QDataStream::Qt_5_15);

//    socketStream.startTransaction();
//    socketStream >> rxBuff;

//    while (!socketStream.commitTransaction()) {
//        socketStream >> rxBuff;
//    }
//    qCDebug(logDebug()) << Qt::hex << "TCP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
//    emit read_data(rxBuff);
}
