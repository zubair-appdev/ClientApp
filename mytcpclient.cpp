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
    static QFile *file = nullptr;
    static QByteArray buffer;
    const QByteArray endMarker = "###_FILE_DONE_###";
    static QString globalFileSize;
    static quint64 bytesWritten;
    static quint64 loadCounter = 0;

    QByteArray data = mySocket->readAll();
    buffer.append(data);


    // If it's a normal server message
    if (buffer.startsWith("*Server Message*")) {
        qDebug() << "Message From Server " << buffer;
        emit guiData(QString::fromUtf8(buffer));

        QString nowTime = QDateTime::currentDateTime().toString("[hh:mm:ss:zzz dd/MM/yyyy]");
        mySocket->write("Client Received Your Message ! " + nowTime.toUtf8());

        buffer.clear();
        return;
    }

    // Check for header
    if (!file && buffer.startsWith("FILE_*_"))
    {
        int headerEnd = buffer.indexOf("!!#!!");
        if (headerEnd != -1) {
            QByteArray headerBytes = buffer.left(headerEnd);
            buffer.remove(0, headerEnd + 5); // drop header part

            QString header = QString::fromUtf8(headerBytes);
            QList<QString> parts = header.split("_*_");
            QString fileName = parts[1];
            QString fileSize = parts[2];
            globalFileSize = fileSize;

            emit guiData("Receiving file : "+fileName+" with size (bytes) "+fileSize);

            file = new QFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + fileName);
            if (!file->open(QIODevice::WriteOnly)) {
                delete file;
                file = nullptr;
                buffer.clear();
                return;
            }
        }
    }

    // If end marker found
    int markerIndex = buffer.indexOf(endMarker);

    if (markerIndex != -1)
    {
        if (file) {
            file->write(buffer.left(markerIndex));  // write everything before marker
            file->flush();
            file->close();
            delete file;
            file = nullptr;
            bytesWritten = 0;
        }
        emit guiData("DATA_SIZE :  File Completely Received!");
        emit guiData("File completely received!");

        buffer.clear(); // reset for next transfer
    }
    else
    {
        // Write intermediate chunks
        if (file) {
            loadCounter++;
            file->write(buffer);
            bytesWritten+=buffer.size();
            buffer.clear(); // keep buffer clean, only hold partial data
            if(loadCounter % 500 == 0)
            {
                emit guiData("DATA_SIZE : "+QString::number(bytesWritten)+" / "+globalFileSize);
            }
        }
    }
}






