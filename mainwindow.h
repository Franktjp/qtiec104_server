#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "qiec104.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui;
    QIec104 iec;
    QTimer* logTimer;

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    int getCurrentRow(QTableWidget*);   // get the current row of table widget


private slots:
    void slotInitServerSuccess();
    void slotInitServerError();
    void slotNewConnection();
    void slotLogTimerTimeout(); // timeout for log timer
    void slotPbClearClicked();  // clear the contents of log tableWidget
    void slotClientTcpDisconnect();   // client socket disconnected

signals:
    // TODO:
};
#endif  // MAINWINDOW_H
