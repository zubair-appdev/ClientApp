#ifndef EVENTSPY_H
#define EVENTSPY_H

#include <QObject>
#include <QEvent>
#include <QDebug>
#include <QDateTime>

class EventSpy : public QObject
{
    Q_OBJECT

public:
    explicit EventSpy(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // EVENTSPY_H
