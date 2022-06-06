#ifndef QIEC104_H
#define QIEC104_H

#include <QObject>
#include <QTimer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QNetworkInterface>

#include "iec_base.h"

class QIec104 : public QObject, public iec_base {
    Q_OBJECT
public:
    explicit QIec104(QObject* parent = 0);
    ~QIec104();

    QTcpSocket* cli;    // 连接socket（与客户端）
    QTcpServer* tcpServer;  // 监听socket


    // TODO: 定时器还有用吗
    QTimer* tm;  // tmKeepAlive定时器(1 second)

private:
    bool end;           // 是否终止
    bool allowConnect;  //

private:

public:
    //
    void terminate();

    void enableConnect();
    void disableConnect();

    void initServer();  // init the server

private:
    // override base class virtual functions
    int readTCP(char* buf, int size);
    void sendTCP(const char* buf, int size);

    void tcpConnect();
    void tcpDisconnect();

    int bytesAvailable();
    void waitForReadyRead(int bytes, int msecs);

    void dataIndication(struct iec_obj* /* obj */,
                        unsigned int /* numpoints */);

    // TODO
    void udpConnect();
    void udpDisconnect();

signals:
    void signalInitServerSuccess();
    void signalInitServerError();
    void signalTcpCpnnect();
    void signalTcpDisconnect();
    void signalDataIndication(struct iec_obj*, unsigned int);
    void signalNewConnection(); // new client connection

public slots:
    void slotTcpDisconnect();

private slots:
    void slotTcpConnect();
    void slotNewConnection();   // connect client
    void slotTcpReadyRead();  // ready to read via tcp socket
    void slotTcpError(QAbstractSocket::SocketError err);
    void slotTimeOut();  // when the alarm times out
};

#endif  // QIEC104_H
