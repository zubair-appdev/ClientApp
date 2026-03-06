#include "eventspy.h"

EventSpy::EventSpy(QObject *parent) : QObject(parent)
{

}

bool EventSpy::eventFilter(QObject *obj, QEvent *event)
{
    //    if(event->type() == QEvent::SockAct)
    //    {
    //        // For only network events
    //        qDebug()<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << " [EVENT]" << event->type() << "Object:" << obj->metaObject()->className();
    //    }

    qDebug()<< QDateTime::currentDateTime().toString("hh:mm:ss:zzz") << " [EVENT]" << event->type() << "Object:" << obj->metaObject()->className();
    return QObject::eventFilter(obj, event);
}
