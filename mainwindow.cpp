#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mytcpclient.h"  // Include the header for myTcpClient

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Client Application");

    // Create myTcpClient as a member object
    myClient = new myTcpClient(this);  // Dynamically allocate client object


    connect(myClient,&myTcpClient::guiData,this,&MainWindow::recvGuiData);



    // Create a regex for a valid IPv4 address
    QRegularExpression ipRegex(
                R"(^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])$)");

    // Create a validator based on the regex
    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);

    // Apply the validator to the QLineEdit
    ui->lineEdit_ipAddr->setValidator(ipValidator);
    ui->lineEdit_ipAddr->setPlaceholderText("127.0.0.1");

    clientConnected = false;


}

MainWindow::~MainWindow()
{
    delete ui;
    delete myClient;  // Clean up the client object
}

void MainWindow::recvGuiData(const QString &recvData)
{
    if(recvData.startsWith("*Server Message*"))
    {
        // Display the message in red color
        ui->textEdit_client->append("<span style='color: red;'>" + recvData + "</span>");
    }
    else if(recvData.startsWith("DATA_SIZE"))
    {
        ui->label_Status->setText(recvData);
    }
    else
    {
        ui->textEdit_client->append(recvData);
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    QString dynamicMessage = ui->textEdit_sendMessage->toPlainText();
    QString nowTime = QDateTime::currentDateTime().toString("[hh:mm:ss:zzz dd/MM/yyyy]");
    myClient->sendMessage(dynamicMessage+"   "+nowTime);
    ui->textEdit_sendMessage->clear();
}

void MainWindow::on_pushButton_connect_clicked()
{
    if(clientConnected == false)
    {
        clientConnected = true;
        QString ipAddress = ui->lineEdit_ipAddr->text().trimmed();
        quint16 port = static_cast<quint16>(ui->spinBox_port->value());
        this->port = port;

        // Check if IP address or port is in default state
        if (ipAddress.isEmpty() && port == 1024) {
            ipAddress = "127.0.0.1";  // Default to loopback
            port = 1024;             // Default port
            QMessageBox::information(this, "Default Connection",
                                     "IP and port not set. Connecting to default:\n"
                                     "IP: 127.0.0.1\nPort: 1024");
        }


        // Attempt to connect to the server
        myClient->connectToServer(ipAddress, port);
    }
    else
    {
        QMessageBox::information(this,"Already Connected","Client is already connected to port "
                                 +QString::number(port) + " .Please restart application to connect new port");
    }

}

