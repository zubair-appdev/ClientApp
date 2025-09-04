#ifndef MYTCPCLIENT_H
#define MYTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QDateTime>
#include <QStandardPaths>
#include <QDataStream>
#include <QFile>
#include <QDir>
#include <QBuffer>
#include <QEventLoop>
#include <QTimer>
#include <QApplication>



class myTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit myTcpClient(QObject *parent = nullptr);
    void connectToServer(const QString &host,quint16 port);
    void sendMessage(const QString &message);

    inline void lightPause()
    {
        QEventLoop loop;
        QTimer::singleShot(1, &loop, &QEventLoop::quit);
        loop.exec();
        QApplication::processEvents();  // Keep UI healthy
    }

signals:
    void guiData(const QString& dataGui);

private slots:
    void onConnected();
    void onServerMessage();

private:
    QTcpSocket *mySocket;

};

#endif // MYTCPCLIENT_H
