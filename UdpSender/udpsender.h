// myudp.h

#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QObject>
#include <QtNetwork/QUdpSocket>


class UdpSender : public QObject
{
    Q_OBJECT
public:
    explicit UdpSender(QObject *parent = 0);

    void odesliRaw(QString adresa, QString vstup, quint16 port);
signals:
  void prijataData(QString data);

public slots:
    void readyRead();

private slots:
    void slotZmenaStavu(QAbstractSocket::SocketState socketState);
protected:
    QUdpSocket *socket;


    //   bool bindPorts(QHostAddress addr, qint16 port);
};

#endif // UDPSENDER_H
