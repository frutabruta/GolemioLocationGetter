// myudp.cpp

#include "udpsender.h"

UdpSender::UdpSender(QObject *parent) :
    QObject(parent)
{
    // create a QUDP socket
    socket = new QUdpSocket(this);

    // The most common way to use QUdpSocket class is
    // to bind to an address and port using bind()
    // bool QAbstractSocket::bind(const QHostAddress & address,
    //     quint16 port = 0, BindMode mode = DefaultForPlatform)
    socket->bind(QHostAddress("192.168.0.131"), 10001);  //musi byt vzdy stejne jako IP!
    //socket->bind(QHostAddress("127.0.0.1"), 10001);
    //  bindPorts()
    //socket->bind(QHostAddress::LocalHost, 61074);
    //   connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    connect(socket, &QUdpSocket::readyRead, this, &UdpSender::readyRead);
    connect(socket, &QUdpSocket::stateChanged, this, &UdpSender::slotZmenaStavu);

}

/*
bool UdpSender::bindPorts(QHostAddress addr, qint16 port)
{
    socket_streamingclient->abort();

    if(socket_streamingclient->bind( QHostAddress::Any, ROPA_STREAMPORT,
                                     QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) )
    {
        connect(socket_streamingclient, &QUdpSocket::readyRead, this, &UdpSender::messageReady);
    }

    return true;
}*/








void UdpSender::odesliRaw(QString adresa,QString  vstup, quint16 port)
{

    qDebug()<<Q_FUNC_INFO<<" "<<port;
    socket->writeDatagram(vstup.toLatin1(), QHostAddress(adresa), port);
}


void UdpSender::readyRead()
{
    qDebug()<<Q_FUNC_INFO;
    // when data comes in
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    // qint64 QUdpSocket::readDatagram(char * data, qint64 maxSize,
    //                 QHostAddress * address = 0, quint16 * port = 0)
    // Receives a datagram no larger than maxSize bytes and stores it in data.
    // The sender's host address and port is stored in *address and *port
    // (unless the pointers are 0).

    /*
    while (udpSocket->hasPendingDatagrams()) {
            QNetworkDatagram datagram = udpSocket->receiveDatagram();
            processTheDatagram(datagram);
        }*/

    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    qDebug() << "Message from: " << sender.toString();
    qDebug() << "Message port: " << senderPort;
    qDebug() << "Message: " << buffer;

    buffer= buffer.replace(char(0x00),char(0x20));
    QString vysledek=QString::fromUtf8(buffer);
    qDebug() << "Message2: " << vysledek;
    emit prijataData(vysledek.toHtmlEscaped());
}




void UdpSender::slotZmenaStavu(QAbstractSocket::SocketState socketState)
{
    qDebug()<<Q_FUNC_INFO;
    qDebug()<<socketState;
}

