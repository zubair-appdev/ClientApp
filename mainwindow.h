
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mytcpclient.h"
#include <QTimer>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void recvGuiData(const QString& recvData);

    void on_pushButton_send_clicked();

    void on_pushButton_connect_clicked();

private:
    Ui::MainWindow *ui;

    myTcpClient *myClient;

    bool clientConnected;

    int port;
};
#endif // MAINWINDOW_H
