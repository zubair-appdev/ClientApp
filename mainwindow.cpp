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

    // Steps
    normalStep = ui->groupBox_gamePad->width() * 0.01;
    boostStep = ui->groupBox_gamePad->width() * 0.2;

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

    //Special balls

    // 🔥 Clear special balls
    for(QLabel* s : specialBalls)
        s->deleteLater();

    specialBalls.clear();

    bulletCount = 0;
    myLocalBullets = 0;

    ui->label_bullets_server->setText("0");
    ui->label_bullets->setText("0");

    boostSteps = 0;
    serverBoostSteps = 0;
    ui->label_steps->setText("0");
    ui->label_steps_server->setText("0");

    boostInitialized = false;
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

    // laser sound
    laserSound = new QSoundEffect(this);
    laserSound->setSource(QUrl("qrc:/new/prefix1/mixkit-laser-cannon-shot-1678.wav"));
    laserSound->setVolume(0.9);
}

void MainWindow::checkSpecialCollision()
{
    QRect playerRect =
        ui->label_gameBox->geometry();

    qDebug() << "CLIENT checkSpecialCollision called";
    qDebug() << "specialBalls size:" << specialBalls.size();

    for(int i = 0; i < specialBalls.size(); i++)
    {
        QLabel *ball =
            specialBalls[i];

        if(playerRect.intersects(
                ball->geometry()))
        {
            int id =
                ball->property(
                    "specialId"
                ).toInt();

            qDebug()
                << "CLIENT collected special ID:"
                << id;

            // Remove ball locally

            ball->deleteLater();

            specialBalls.removeAt(i);
            eatSound->play();
            // Increase bullets

            myLocalBullets += 4;

            ui->label_bullets
                ->setText(
                    QString::number(
                        myLocalBullets
                    )
                );

            // Inform server

            QString msg =
                QString(
                    "@@@CLIENT_BULLETS@@@_%1_%2"
                )
                .arg(myLocalBullets)
                .arg(id);

            qDebug()
                << "Sending bullets to server and id:"
                << myLocalBullets <<" "<<id;

            myClient->sendMessage(msg);

            break;
        }
    }
}

void MainWindow::drawLaser(
    int direction,
    bool isClient
)
{
    QWidget *pad =
        ui->groupBox_gamePad;

    QLabel *player;

    // Decide whose laser

    if(isClient)
        player = ui->label_gameBox_enemy;
    else
        player = ui->label_gameBox;

    QPoint pos =
        player->pos();

    int length =
        pad->width() * 0.4;

    int thickness = 6;

    QFrame *laser =
        new QFrame(pad);

    laser->setStyleSheet(
        "background:red;"
    );

    // RIGHT

    if(direction == 0)
    {
        laser->setGeometry(
            pos.x() + player->width(),
            pos.y() +
            player->height()/2,
            length,
            thickness
        );
    }

    // LEFT

    else if(direction == 1)
    {
        laser->setGeometry(
            pos.x() - length,
            pos.y() +
            player->height()/2,
            length,
            thickness
        );
    }

    // UP

    else if(direction == 2)
    {
        laser->setGeometry(
            pos.x() +
            player->width()/2,
            pos.y() - length,
            thickness,
            length
        );
    }

    // DOWN

    else if(direction == 3)
    {
        laser->setGeometry(
            pos.x() +
            player->width()/2,
            pos.y() +
            player->height(),
            thickness,
            length
        );
    }

    laser->show();

    laserSound->play();

    // Remove after 200 ms
    QTimer::singleShot(
        200,
        laser,
        &QFrame::deleteLater
    );
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QLabel *box = ui->label_gameBox;
    QWidget *pad = ui->groupBox_gamePad;

    int step;

    if(useBoost)
    {
        step = boostStep;   // 0.2

        useBoost = false;   // reset after one move
    }
    else
    {
        step = normalStep;  // 0.01
    }

    QPoint pos = box->pos();

    if(event->key() == Qt::Key_X)
    {
        if(myLocalBullets <= 0)
        {
            qDebug() << "No bullets to shoot";
            return;
        }

        // Reduce bullets locally

        myLocalBullets--;

        ui->label_bullets->setText(
            QString::number(myLocalBullets)
        );

        // Draw locally

        drawLaser(
            lastDirection,
            false
        );

        // Send shoot command

        QString msg =
            QString(
                "@@@SHOOT@@@_%1_%2"
            )
            .arg(lastDirection)
            .arg(myLocalBullets);

        myClient->sendMessage(msg);

        qDebug()
            << "Shot fired direction:"
            << lastDirection;

        return;
    }

    // Boost Steps Code (CLIENT)

    if(event->key() == Qt::Key_Z)
    {
        if(boostSteps <= 0)
        {
            qDebug()
                << "Client: No boost steps remaining";

            return;
        }

        boostSteps--;

        useBoost = true;

        ui->label_steps->setText(
            QString::number(boostSteps)
        );

        qDebug()
            << "Client boost activated. Remaining:"
            << boostSteps;

        // 🔥 Inform server

        QString msg =
            QString(
                "@@@CLIENT_BOOST_STEPS@@@_%1"
            ).arg(boostSteps);

        myClient->sendMessage(msg);

        return;
    }


    switch (event->key())
    {
    case Qt::Key_Left:
        pos.rx() -= step;
        lastDirection = LEFT;
        break;

    case Qt::Key_Right:
        pos.rx() += step;
        lastDirection = RIGHT;
        break;

    case Qt::Key_Up:
        pos.ry() -= step;
        lastDirection = UP;
        break;

    case Qt::Key_Down:
        pos.ry() += step;
        lastDirection = DOWN;
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
    checkSpecialCollision();

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
            qDebug()<<t<<" tz";

            int minutes = t / 60;
            int seconds = t % 60;

            // =========================
            // INITIALIZE CLIENT BOOST STEPS
            // =========================

            if(!boostInitialized)
            {
                int totalMinutes =
                    (t+1) / 60;   // derive from server time

                boostSteps =
                    totalMinutes * 12;

                ui->label_steps->setText(
                    QString::number(boostSteps)
                );

                boostInitialized = true;

                qDebug()
                    << "Client boost steps initialized:"
                    << boostSteps;
            }


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
    else if(recvData.contains("@@@SPECIAL@@@"))
    {
        QString data = recvData;

        data.replace("*Server Message*: ", "");

        QStringList packets =
            data.split("@@@SPECIAL@@@");

        for(const QString &p : packets)
        {
            if(p.trimmed().isEmpty())
                continue;

            QStringList parts =
                p.split("_");

            if(parts.size() < 4)
                continue;

            int id =
                parts[1].toInt();

            double rx =
                parts[2].toDouble();

            double ry =
                parts[3].toDouble();

            QWidget *pad =
                ui->groupBox_gamePad;

            int x =
                rx * pad->width();

            int y =
                ry * pad->height();

            QLabel *ball =
                new QLabel(pad);

            ball->resize(16,16);

            ball->setStyleSheet(
                "background:black;"
                "border-radius:8px;"
            );

            ball->move(x,y);

            ball->show();

            ball->setProperty(
                "specialId",
                id
            );

            specialBalls.append(ball);
        }
    }
    else if(recvData.contains("@@@SPECIAL_REMOVE@@@"))
    {
        QString data = recvData;

        data.replace(
            "*Server Message*: ",
            ""
        );

        QStringList parts =
            data.split("_");

        if(parts.size() < 2)
            return;

        int id =
            parts[2].toInt();

        for(int i = 0;
            i < specialBalls.size();
            i++)
        {
            QLabel *ball =
                specialBalls[i];

            if(ball->property(
                   "specialId"
               ).toInt() == id)
            {
                ball->deleteLater();

                specialBalls.removeAt(i);

                break;
            }
        }
    }
    else if(recvData.contains("@@@BULLETS@@@"))
    {
        int count =
            recvData.split("_")[1]
            .toInt();

        bulletCount = count;

        ui->label_bullets_server->setText(
            QString::number(
                bulletCount
            )
        );
    }
    else if(recvData.contains("@@@DRAW_LASER_SERVER@@@"))
    {
        QString data = recvData;

        data.replace(
            "*Server Message*: ",
            ""
        );

        QStringList parts =
            data.split("_");


        if(parts.size() < 5)
            return;

        int direction =
            parts[3].toInt();

        int bullets =
            parts[4].toInt();

        // Update server bullets label on client

        bulletCount = bullets;

        ui->label_bullets_server
            ->setText(
                QString::number(
                    bulletCount
                )
            );

        qDebug()
            << "Client drawing server laser direction:"
            << direction;

        drawLaser(
            direction,
            true
        );
    }
    else if(recvData.contains("@@@BOOST_STEPS@@@"))
    {
        QString data = recvData;

        data.replace(
            "*Server Message*: ",
            ""
        );

        QStringList parts =
            data.split("_");

        if(parts.size() < 3)
            return;

        int steps =
            parts[2].toInt();

        // Store server steps separately

        serverBoostSteps = steps;

        ui->label_steps_server
            ->setText(
                QString::number(serverBoostSteps)
            );

        qDebug()
            << "Client updated server boost steps:"
            << serverBoostSteps;
    }
    //PTP Code Start
    else if(recvData.contains("@@@SYNC@@@"))
    {
        QStringList parts = recvData.split("_");

        t1 = parts[1].toLongLong();

        t2 = now_us();

        qDebug()
                << "SYNC received"
                << "t1:" << t1
                << "t2:" << t2;

        // IMPORTANT — store t3
        t3 = now_us();

        QString msg =
                QString("@@@DELAY_REQ@@@_%1")
                .arg(t3);

        myClient->sendMessage(msg);
    }
    else if(recvData.contains("@@@DELAY_RESP@@@"))
    {
        QString data = recvData;

        // Remove server prefix
        data.replace("*Server Message*: ", "");

        QStringList parts = data.split("_");

        if(parts.size() < 3)
            return;

        // Capture t4
        t4 = parts[2].toLongLong();

        // =========================
        // CALCULATE OFFSET & DELAY
        // =========================

        qint64 offset =
                ((t2 - t1) -
                 (t4 - t3)) / 2;

        qint64 delay =
                ((t2 - t1) +
                 (t4 - t3)) / 2;

        // =========================
        // CURRENT MEASUREMENT TIME
        // =========================

        QString now =
            QDateTime::currentDateTime()
            .toString("hh:mm:ss.zzz");

        // =========================
        // BUILD MESSAGE
        // =========================

        QString msg;

        msg += "[" + now + "]\n";

        msg += "==============================\n";

        msg += QString(
               "t1 (Master send time)      : %1 us\n")
               .arg(t1);

        msg += QString(
               "t2 (Client receive time)   : %1 us\n")
               .arg(t2);

        msg += QString(
               "t3 (Client send request)   : %1 us\n")
               .arg(t3);

        msg += QString(
               "t4 (Master receive time)   : %1 us\n")
               .arg(t4);

        msg += "\n";

        msg += QString(
               "Offset (Clock difference)  : %1 us")
               .arg(offset);

        msg += QString(
               "  (%1 ms)\n")
               .arg(offset / 1000.0, 0, 'f', 3);

        msg += QString(
               "Delay (Network latency)    : %1 us")
               .arg(delay);

        msg += QString(
               "  (%1 ms)\n")
               .arg(delay / 1000.0, 0, 'f', 3);

        msg += "==============================\n";

        // =========================
        // DISPLAY IN UI
        // =========================

        ui->plainTextEdit_ptp->appendPlainText(msg);

        // =========================
        // AUTO SCROLL
        // =========================

        QTextCursor cursor =
                ui->plainTextEdit_ptp->textCursor();

        cursor.movePosition(QTextCursor::End);

    }
    //PTP Code End
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

void MainWindow::on_actionPtp_page_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->page_ptp);
}

void MainWindow::on_pushButton_back_ptp_clicked()
{
    ui->plainTextEdit_ptp->clear();
    ui->stackedWidget->setCurrentWidget(ui->page_main);
}


void MainWindow::on_actionHelp_triggered()
{
    QString helpText;

    helpText += "================ GAME RULES ================\n\n";

    helpText += "1. STARTING THE GAME\n";
    helpText += "   - Click the 'Generate Eatables' button on the Server to start the game.\n";
    helpText += "   - Use the Timer SpinBox to set the game duration (1 to 10 minutes).\n";
    helpText += "   - Once started, the timer will begin counting down.\n\n";

    helpText += "2. OBJECTIVES\n";
    helpText += "   - Collect orange eatables to increase your score.\n";
    helpText += "   - Avoid getting hit by the opponent's laser.\n";
    helpText += "   - The player with the higher score when the timer ends wins.\n\n";

    helpText += "3. SPECIAL ITEMS (BLACK BALLS)\n";
    helpText += "   - Special black balls appear periodically during gameplay.\n";
    helpText += "   - Collecting a black ball gives you 4 bullets.\n";
    helpText += "   - Bullets are required to shoot lasers.\n\n";

    helpText += "4. SHOOTING LASER\n";
    helpText += "   - Press arrow keys to choose direction.\n";
    helpText += "   - Press 'X' to shoot a laser in that direction.\n";
    helpText += "   - Each shot consumes 1 bullet.\n";
    helpText += "   - If a laser hits the opponent, their score decreases by 5 points.\n\n";

    helpText += "5. BOOST STEPS (SPEED BOOST)\n";
    helpText += "   - You receive 12 boost steps per minute of gameplay.\n";
    helpText += "   - Press 'Z' to use one boost step.\n";
    helpText += "   - The next movement will be faster than normal.\n";
    helpText += "   - Each boost can be used only once per key press.\n";
    helpText += "   - When boost steps reach zero, only normal movement is available.\n\n";

    helpText += "6. SCORE AND STATUS INDICATORS\n";
    helpText += "   - Blue Label   : Player Score\n";
    helpText += "   - Red Label    : Enemy Score\n";
    helpText += "   - Grey Label   : Player Bullets\n";
    helpText += "   - Yellow Label : Enemy Bullets\n";
    helpText += "   - Green Label  : Player Boost Steps\n";
    helpText += "   - Pink Label   : Enemy Boost Steps\n";
    helpText += "   - Timer Label  : Remaining Game Time\n\n";

    helpText += "7. GAME END\n";
    helpText += "   - The game automatically ends when the timer reaches 00:00.\n";
    helpText += "   - The player with the higher score is declared the winner.\n";
    helpText += "   - All scores, bullets, boost steps, and objects reset for the next game.\n\n";

    helpText += "============================================";

    // Create dialog
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Game Help");
    dialog->resize(500, 400);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Scrollable text area
    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    textEdit->setText(helpText);

    layout->addWidget(textEdit);

    dialog->setLayout(layout);

    dialog->exec();
}
