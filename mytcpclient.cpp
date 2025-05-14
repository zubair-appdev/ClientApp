#include "mytcpclient.h"

myTcpClient::myTcpClient(QObject *parent) : QObject(parent)
{
    mySocket = new QTcpSocket(this);

    connect(mySocket,&QTcpSocket::connected,this,&myTcpClient::onConnected);

    connect(mySocket,&QTcpSocket::readyRead,this,&myTcpClient::onServerMessage);

}

void myTcpClient::connectToServer(const QString &host, quint16 port)
{
    qDebug()<<"Connecting To Server..... "<<host<<" "<<port;
    emit guiData("Connecting To Server..... "+host+" "+QString::number(port));

    mySocket->connectToHost(host,port);
}

void myTcpClient::sendMessage(const QString &message)
{
    // Check if the socket is already connected
    if (mySocket->state() == QTcpSocket::ConnectedState)
    {
        qDebug() << "Socket is connected, sending message...";
        emit guiData("sending message... "+message);

        mySocket->write(message.toUtf8()); // Sends the message in bytes
    }
    else
    {
        qDebug() << "Not Connected To Server";
        emit guiData("Not Connected To Server");

        qDebug() << "Socket state: " << mySocket->state();
    }

}

void myTcpClient::onConnected()
{
    qDebug() << "Connected To Server";
    emit guiData( "Connected To Server : "+mySocket->peerAddress().toString()+"  "+QString::number(mySocket->peerPort()));
}


void myTcpClient::onServerMessage()
{
    QByteArray message = mySocket->readAll();
    qDebug()<<"Message From Server "<<message;
    emit guiData(message);
    QString nowTime = QDateTime::currentDateTime().toString("[hh:mm:ss:zzz dd/MM/yyyy]");
    mySocket->write("Client Received Your Message ! "+nowTime.toUtf8());
}


