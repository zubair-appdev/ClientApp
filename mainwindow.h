
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mytcpclient.h"
#include <QTimer>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <cmath>

#include <QKeyEvent>

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSoundEffect>

#include <chrono>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //PTP IEEE 1588 Code
    static qint64 now_us()
    {
        auto now =
            std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast
        <
            std::chrono::microseconds
        >(now.time_since_epoch()).count();
    }

    void checkFoodCollision();

    void removeFoodById(int id);

    inline void lightPause(quint8 msec)
    {
        QEventLoop loop;
        QTimer::singleShot(msec, &loop, &QEventLoop::quit);
        loop.exec();
        QApplication::processEvents();  // Keep UI healthy
    }

    void resetThings();

    void initializeMusic();
    
    void checkSpecialCollision();

    void drawLaser(
        int direction,
        bool isClient
    );

protected:
    void keyPressEvent(QKeyEvent *event) override;


private slots:
    void recvGuiData(const QString& recvData);

    void on_pushButton_send_clicked();

    void on_pushButton_connect_clicked();

    void on_actionPlay_Game_triggered();

    void on_pushButton_back_clicked();

    void setInitialPos();

    void on_actionSync_720p_triggered();

    void on_actionPtp_page_triggered();

    void on_pushButton_back_ptp_clicked();

    void on_actionHelp_triggered();

private:
    Ui::MainWindow *ui;

    myTcpClient *myClient;

    bool clientConnected;

    int port;

    QVector<QLabel*> eatables;

    QSet<int> eatenFoods;

    int myScore = 0;

    QMediaPlayer *bgMusic;
    QMediaPlaylist *playlist;
    QSoundEffect *eatSound;
    QSoundEffect *laserSound;

    qint64 t1 = 0;
    qint64 t2 = 0;
    qint64 t3 = 0;
    qint64 t4 = 0;

    QVector<QLabel*> specialBalls;
    int bulletCount = 0;
    int myLocalBullets = 0;

    enum Direction
    {
        RIGHT = 0,
        LEFT,
        UP,
        DOWN
    };

    int lastDirection = RIGHT;

    bool boostInitialized = false;

    int boostSteps = 0;          // client's own boosts
    int serverBoostSteps = 0;    // display only

    int normalStep = 0;      // default movement
    int boostStep = 0;      // boosted movement
    bool useBoost = false;   // next move uses boost



};
#endif // MAINWINDOW_H
