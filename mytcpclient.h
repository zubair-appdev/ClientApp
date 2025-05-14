#ifndef MYTCPCLIENT_H
#define MYTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QDateTime>


class myTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit myTcpClient(QObject *parent = nullptr);
    void connectToServer(const QString &host,quint16 port);
    void sendMessage(const QString &message);

signals:
    void guiData(const QString& dataGui);

private slots:
    void onConnected();
    void onServerMessage();

private:
    QTcpSocket *mySocket;

};

#endif // MYTCPCLIENT_H
