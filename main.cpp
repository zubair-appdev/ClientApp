#include "mainwindow.h"
#include "eventspy.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Uncomment below block if you need to see each and every event Qt Produces
    //    EventSpy *spy = new EventSpy();
    //    a.installEventFilter(spy);

    MainWindow w;
    w.show();
    return a.exec();
}
