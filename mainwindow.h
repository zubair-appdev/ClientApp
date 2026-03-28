
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

    qint64 t1 = 0;
    qint64 t2 = 0;
    qint64 t3 = 0;
    qint64 t4 = 0;

};
#endif // MAINWINDOW_H
