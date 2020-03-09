#ifndef QSUBSCRIBER_H
#define QSUBSCRIBER_H

#include "qmessage.h"

#include <QByteArray>
#include <QVariant>

class QSubscriber
{
public:
    QSubscriber(QObject * receiver, QByteArray const & method);

    template<typename T, typename ...Args>
    QSubscriber(T * receiver, void (T::*method)(Args ...args))
    {

    }

public:
    void operator()(QByteArray const & topic, QVariant const & msg);

private:
    QMessageBase::observ_t observ_;
};

#endif // QSUBSCRIBER_H
