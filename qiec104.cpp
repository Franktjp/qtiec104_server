#include "qiec104.h"

QIec104::QIec104(QObject* parent) : QObject(parent) {
    this->end = false;
    this->allowConnect = true;

    this->tm = new QTimer();
    this->log.activateLog();
    this->cli = nullptr;
    this->tcpServer = nullptr;

    // TODO

    connect(this->tm, &QTimer::timeout, this, &QIec104::slotTimeOut);
}

void QIec104::initServer() {
    char buf[100];

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, SERVERPORT)) {
        emit signalInitServerError();
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }

    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    setSlaveIP(ipAddress.toStdString().c_str());
    setSlavePort(tcpServer->serverPort());

    sprintf(buf, "INFO: init server success: %s(%d)", ipAddress.toStdString().c_str(), tcpServer->serverPort());
    log.pushMsg(buf);
    qDebug() << buf;

    connect(this->tcpServer, &QTcpServer::newConnection, this, &QIec104::slotNewConnection);
    emit signalInitServerSuccess();  // success
}


/**
 * @brief QIec104::slotNewConnection - when a new connection is available
 * TODO: 改为通用函数（带一个参数），能够初始化任何一个socket
 */
void QIec104::slotNewConnection() {
    char buf[100];

    cli = tcpServer->nextPendingConnection();

    if (!cli) { // no pending connections
        sprintf(buf, "ERROR: There are no pending connections");
        log.pushMsg(buf);
        qDebug() << buf;
        return;
    }

    connect(cli, &QTcpSocket::connected, this, &QIec104::slotTcpConnect);
    connect(cli, &QTcpSocket::disconnected, this, &QIec104::slotTcpDisconnect);
    connect(cli, &QTcpSocket::readyRead, this, &QIec104::slotTcpReadyRead);
    connect(cli, &QTcpSocket::errorOccurred, this, &QIec104::slotTcpError);

    emit signalNewConnection();
}

void QIec104::slotTcpConnect() {
    char buf[100];
    sprintf(buf, "INFO: client connection is created!");
    log.pushMsg(buf);
    qDebug() << buf;

    // TODO: signal, lower program and option
    //    this->cli->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    //    onTcpConnect();
    //    emit this->signalTcpCpnnect();
}

QIec104::~QIec104() {
    if (cli) {
        delete this->cli;   // cli will be automatically deleted when the tcpServer object is destroyed
    }
    if (tcpServer) {
        delete this->tcpServer;
    }
}

void QIec104::terminate() {
    this->end = true;
    if (cli) {
        cli->close(); // close the connection with client
    }
    if (tcpServer) {
        tcpServer->close();
    }
}

void QIec104::enableConnect() {
    this->allowConnect = true;
}

void QIec104::disableConnect() {
    this->allowConnect = false;
    return;
    // TODO:
    //    if (this->tcp->state() == QAbstractSocket::ConnectedState) {
    //        tcpDisconnect();
    //    }
}

void QIec104::tcpConnect() {
    if (this->end || !this->allowConnect) {
        return;
    }
    // TODO:
    //    this->tcp->close();

    //    if (!this->end && this->allowConnect) {
    //        this->tcp->connectToHost(getSlaveIP(), quint16(getSlavePort()),
    //                                 QIODevice::ReadWrite,
    //                                 QAbstractSocket::IPv4Protocol);
    //        char buf[100];
    //        sprintf(buf, "try to connect ip: %s", getSlaveIP());
    //        log.pushMsg(buf);
    //        qDebug() << buf;
    //    }
}

void QIec104::tcpDisconnect() {
    // TODO:
    //    tcp->close();
}

int QIec104::readTCP(char* buf, int size) {
    int ret;
    if (this->end) {
        return 0;
    }

    // TODO: Now the program is single-thread and we should modify it to multi-thread
    ret = (int)cli->read(buf, size);
    if (ret > 0) {
        return ret;
    }
    return 0;
}

void QIec104::sendTCP(const char* buf, int size) {
    if (this->cli->state() == QAbstractSocket::ConnectedState) {
        this->cli->write(buf, size);
        if (log.isLogging()) {
            showFrame(buf, size, true);
        }
    }
}

void QIec104::slotTcpDisconnect() {
    // TODO: 修改server端实现
    onTcpDisconnect();
    emit signalTcpDisconnect();
}

void QIec104::slotTcpReadyRead() {
    int i = 0;
    if (cli->bytesAvailable() < 6) {
        while (!cli->waitForReadyRead(100) && i < 5) {  // delay 0.1 sec
            // readyRead() signal not be emitted
            ++i;
        }
    }
    packetReadyTCP();
}

void QIec104::slotTcpError(QAbstractSocket::SocketError err) {
    //    if (err != QAbstractSocket::SocketTimeoutError) {   // TODO: 为啥要这样
    // TODO:
    char buf[100];
    sprintf(buf, "socket error : %d(%s)", err,
            cli->errorString().toStdString().c_str());
    log.pushMsg(buf);
    qDebug() << buf;
    //    }
}

void QIec104::slotTimeOut() {
    // TODO:
    //    static unsigned int cnt = 1;
    //    if (!this->end) {
    //        if (!(cnt++ % 5)) {  // 每5s输出一次
    //            if (tcp->state() != QAbstractSocket::ConnectedState &&
    //                this->allowConnect) {
    //                char buf[100];
    //                sprintf(buf, "trying to connect ip: %s", getSlaveIP());
    //                log.pushMsg(buf);
    //                qDebug() << buf;
    //                tcpConnect();
    //            }
    //        }
    //        //        onTimerSecond();  // TODO: iec_base类实现，用于每秒定时处理
    //    }
}

void QIec104::dataIndication(struct iec_obj* obj, unsigned int numpoints) {
    emit signalDataIndication(obj, numpoints);
}

void QIec104::udpConnect() {
    // TODO
}

void QIec104::udpDisconnect() {
    // TODO
}

int QIec104::bytesAvailable() {
    // TODO:
    //    return int(tcp->bytesAvailable());
    return -1;
}

void QIec104::waitForReadyRead(int bytes, int msecs) {
    // TODO:
    //    int i = 0;
    //    while (i < msecs && tcp->bytesAvailable() < bytes) {
    //        tcp->waitForReadyRead(10);
    //        i += 10;
    //    }
}
