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

    // Gaming session
    ui->stackedWidget->setCurrentWidget(ui->page_main);

    // Music
    initializeMusic();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete myClient;  // Clean up the client object
}

void MainWindow::checkFoodCollision()
{
    for(int i=0; i<eatables.size(); i++)
    {
        int id = eatables[i]->property("foodId").toInt();

        // prevent duplicate sending
        if(eatenFoods.contains(id))
            continue;

        if(ui->label_gameBox->geometry().intersects(
                    eatables[i]->geometry()))
        {
            eatenFoods.insert(id);

            // ⭐ instant local feedback
            eatables[i]->hide();

            // ⭐ instant score update (local feel)
            myScore++;
            eatSound->play();
            ui->label_myScore->setText(QString::number(myScore));

            // inform server
            myClient->sendMessage(
                        QString("@@@ATE@@@_%1").arg(id)
                        );

            return;
        }
    }
}

void MainWindow::removeFoodById(int id)
{
    for(int i = 0; i < eatables.size(); i++)
    {
        if(eatables[i]->property("foodId").toInt() == id)
        {
            eatables[i]->deleteLater();
            eatables.remove(i);
            return;
        }
    }
}

void MainWindow::resetThings()
{
    // 🔥 Clear all eatables
    for(QLabel* f : eatables)
        f->deleteLater();

    eatables.clear();
    eatenFoods.clear();
    myScore = 0;

    ui->label_myScore->setText("0");
    ui->label_enemyScore->setText("0");
    ui->label_timer->setText("00:00");
}

void MainWindow::initializeMusic()
{
    bgMusic = new QMediaPlayer(this);

    playlist = new QMediaPlaylist(this);
    playlist->addMedia(QUrl("qrc:/new/prefix1/mixkit-wedding-01-657.mp3"));
    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    bgMusic->setPlaylist(playlist);
    bgMusic->setVolume(60);


    // eating sound
    eatSound = new QSoundEffect(this);
    eatSound->setSource(QUrl("qrc:/new/prefix1/mixkit-video-game-retro-click-237.wav"));
    eatSound->setVolume(0.9);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QLabel *box = ui->label_gameBox;
    QWidget *pad = ui->groupBox_gamePad;

    int step = 5; // movement speed (pixels)

    QPoint pos = box->pos();

    switch (event->key())
    {
    case Qt::Key_Left:
        pos.rx() -= step;
        break;

    case Qt::Key_Right:
        pos.rx() += step;
        break;

    case Qt::Key_Up:
        pos.ry() -= step;
        break;

    case Qt::Key_Down:
        pos.ry() += step;
        break;

    default:
        QMainWindow::keyPressEvent(event);
        return;
    }

    // 🔒 Boundary check (stay inside groupBox)
    int maxX = pad->width() - box->width();
    int maxY = pad->height() -box->height();

    qDebug()<<maxX<<" :maxX "<<maxY<<" :maxY";
    qDebug()<<pos.x()<<" :x direction";
    qDebug()<<pos.y()<<" :y direction";

    pos.setX(qBound(0, pos.x(), maxX));
    int initialY = 0 + pad->height() * 0.01;
    pos.setY(qBound(initialY, pos.y(), maxY));

    box->move(pos);
    checkFoodCollision();

    QString msg = QString("@@@MOVE_CLIENT@@@_%1_%2")
            .arg(pos.x())
            .arg(pos.y());

    myClient->sendMessage(msg);

}


void MainWindow::recvGuiData(const QString &recvData)
{
    // 🔥 Handle merged packets first
        if(recvData.count("*Server Message*: ") > 1)
        {
            qDebug()<<"Recursion Hits : ";
            QStringList packets = recvData.split("*Server Message*: ", Qt::SkipEmptyParts);

            for(const QString &p : packets)
            {
                recvGuiData("*Server Message*: " + p);   // reprocess each packet
            }
            return;
        }

    if(recvData.contains("@@@MOVE_SERVER@@@"))
    {
        QStringList parts = recvData.split("_");

        int x = parts[2].toInt();
        int y = parts[3].toInt();

        ui->label_gameBox_enemy->move(x,y);
        ui->textEdit_client->append("<span style='color: red;'>" + recvData + "</span>");

    }
    else if(recvData.contains("@@@REMOVE@@@"))
    {
        QString data = recvData;

        // remove prefix if present
        data.replace("*Server Message*: ", "");

        QStringList packets = data.split("@@@REMOVE@@@");

        for(const QString &p : packets)
        {
            if(p.trimmed().isEmpty()) continue;

            QStringList parts = p.split("_");
            if(parts.size() < 2) continue;

            int id = parts[1].toInt();

            removeFoodById(id);
        }
    }
    else if(recvData.contains("@@@SCORE_SERVER@@@"))
    {
        int score = recvData.split("_")[2].toInt();
        ui->label_enemyScore->setText(QString::number(score));
    }

    else if(recvData.contains("@@@SCORE_CLIENT@@@"))
    {
        int score = recvData.split("_")[2].toInt();
        ui->label_myScore->setText(QString::number(score));
    }
    else if(recvData.contains("@@@FOOD@@@"))
    {
        QString data = recvData;

        data.replace("*Server Message*: ", "");

        QStringList packets = data.split("@@@FOOD@@@");

        for(const QString &p : packets)
        {
            if(p.trimmed().isEmpty()) continue;

            QStringList parts = p.split("_");
            if(parts.size() < 4) continue;

            int id = parts[1].toInt();
            double rx = parts[2].toDouble();
            double ry = parts[3].toDouble();

            QWidget *pad = ui->groupBox_gamePad;

            int x = rx * pad->width();
            int y = ry * pad->height();

            QLabel *food = new QLabel(pad);
            food->resize(14,14);
            food->setStyleSheet("background:orange; border-radius:7px;");
            food->move(x,y);
            food->show();

            // ⭐ store ID
            food->setProperty("foodId", id);

            eatables.append(food);
        }
    }
    else if(recvData.contains("@@@TIME@@@"))
    {
        QString data = recvData;

        // remove any server prefix
        data.replace("*Server Message*: ", "");

        // split in case multiple packets are merged
        QStringList packets = data.split("@@@TIME@@@");

        for(const QString &p : packets)
        {
            if(p.trimmed().isEmpty()) continue;

            QStringList parts = p.split("_");
            if(parts.size() < 2) continue;

            int t = parts[1].toInt();

            int minutes = t / 60;
            int seconds = t % 60;

            ui->label_timer->setText(
                        QString("%1:%2")
                        .arg(minutes,2,10,QChar('0'))
                        .arg(seconds,2,10,QChar('0')));
        }
    }
    else if(recvData.contains("@@@GAME_OVER@@@"))
    {
        qDebug() << "Game Over In Client ####";

        bgMusic->stop();

        QRegularExpression reGameOver("@@@GAME_OVER@@@_(.+)");
        QRegularExpressionMatchIterator it = reGameOver.globalMatch(recvData);

        while(it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            QString result = match.captured(1).trimmed();


            // 🔥 Show result
            QMessageBox::information(this, "GAME OVER", result);

            // 🔥 Reset scores
            myScore = 0;

            ui->label_myScore->setText("0");
            ui->label_enemyScore->setText("0");
            ui->label_timer->setText("00:00");

            // 🔥 Clear foods
            for(QLabel* f : eatables)
                f->deleteLater();

            eatables.clear();
        }

        resetThings();
    }
    else if(recvData.startsWith("*Server Message*"))
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
        ui->textEdit_client->append("<span style='color: black;'>" + recvData + "</span>");
    }

    // Always scroll to bottom
    QTextCursor cursor = ui->textEdit_client->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEdit_client->setTextCursor(cursor);
    ui->textEdit_client->ensureCursorVisible();
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


void MainWindow::on_actionPlay_Game_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->page_game);

    bgMusic->play();

    resetThings();

    // Enable key handling
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();

    setInitialPos();
}

void MainWindow::on_pushButton_back_clicked()
{
    this->setFixedSize(700,600);

    resetThings();

    bgMusic->stop();

    ui->stackedWidget->setCurrentWidget(ui->page_main);

    // Disable key handling
    this->setFocusPolicy(Qt::NoFocus);
}

void MainWindow::setInitialPos()
{
    //Setting positions
    QWidget *pad = ui->groupBox_gamePad;
    QLabel *me = ui->label_gameBox;
    QLabel *enemy = ui->label_gameBox_enemy;

    // 🔹 compute vertical center
    int centerY = (pad->height() - me->height()) / 2;

    // 🔹 place at extremes
    me->move(0, centerY);   // LEFT extreme
    enemy->move(pad->width() - enemy->width(), centerY); // RIGHT extreme
}

void MainWindow::on_actionSync_720p_triggered()
{
    this->setFixedSize(1280,720);
}
