#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // set table widget
    ui->twConnected->setColumnCount(2); // 设置列数
    ui->twConnected->setHorizontalHeaderLabels(QStringList() << "IP地址" << "端口");    // 设置水平表头





    connect(&iec, &QIec104::signalInitServerSuccess, this, &MainWindow::slotInitServerSuccess);
    connect(&iec, &QIec104::signalInitServerError, this, &MainWindow::slotInitServerError);
    connect(&iec, &QIec104::signalNewConnection, this, &MainWindow::slotNewConnection);
    connect(ui->pbInitServer, &QPushButton::clicked, &iec, &QIec104::initServer);






}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::slotInitServerSuccess() {
    char buf[100];
    sprintf(buf, "The server is running on\n\nIP: %s\nport: %d", iec.getSlaveIP(), iec.getSlavePort());
    qDebug() << buf;
    iec.log.pushMsg(buf);

    QString s;
    QTextStream(&s) << iec.getSlavePort();
    ui->leIPAddr->setText(iec.getSlaveIP());
    ui->lePort->setText(s);

    ui->pbInitServer->setEnabled(false);    // 禁用
}

void MainWindow::slotInitServerError() {
    char buf[100];
    sprintf(buf, "Init Server: Unable to start the server: %s.",
            iec.tcpServer->errorString().toStdString().c_str());
    iec.log.pushMsg(buf);
    qDebug() << buf;
}

void MainWindow::slotNewConnection() {
    char buf[100];
    sprintf(buf, "%s: connect client ip: %s, port: %d", Q_FUNC_INFO, iec.cli->peerAddress().toString().toStdString().c_str(), iec.cli->peerPort());
    iec.log.pushMsg(buf);
    qDebug() << buf;

    // set table widget item
    int nextRows = getCurrentRow(ui->twConnected) + 1;
    ui->twConnected->setRowCount(nextRows + 1);
    ui->twConnected->setItem(nextRows, 0, new QTableWidgetItem(iec.cli->peerAddress().toString()));
    ui->twConnected->setItem(nextRows, 1, new QTableWidgetItem(QString::number(iec.cli->peerPort())));
}


int MainWindow::getCurrentRow(QTableWidget* tw) {
    for (int i=0; i<tw->rowCount(); i++) {
        for (int j=0; j<tw->columnCount(); j++) {
            if (tw->cellWidget(i,j) == tw->focusWidget()) {
                return i;
            }
        }
    }
    return -1;
}
