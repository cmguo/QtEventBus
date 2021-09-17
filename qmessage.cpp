#include "qmessage.h"

QMessageBase::QMessageBase(bool external, bool stick, QByteArray const & topic)
    : external_(external)
    , stick_(stick)
    , topic_(topic)
{
}

QByteArray const & QMessageBase::topic() const
{
    return topic_;
}

QEventQueue *QMessageBase::queue() const
{
    return queue_;
}

QtPromise::QPromise<QVector<QVariant>> QMessageBase::publish(const QVariant &msg)
{
    return publish(nullptr, msg);
}

bool QSimpleMessage::subscribe(const QObject *c, observ_t o, bool recv_stick)
{
    QObject::connect(this, &QMessageBase::on_message, c,
                     [this, o](QMessageResultPointer const & e) {
        e.invoke<QVariant, QVariant>([=](auto & m) { return o(topic_, m); });
    });
    if (last_.isValid() && recv_stick) {
        // send stick message on specific context
        auto cn = QObject::connect(this, &QMessageBase::on_stick_message, c,
                                   [this, o](QMessageResultPointer const & e){
            e.invoke2<QVariant, QVariant>([=](auto & m) { return o(topic_, m); });
        });
        emit on_stick_message(QMessageResultPointer(last_));
        disconnect(c, SIGNAL(on_stick_message(QMessageResultPointer)));
    }
    return external_ && !topic_.isEmpty()
            && receivers(SIGNAL(on_message(QMessageResultPointer))) == 1;
}

bool QSimpleMessage::unsubscribe(const QObject *c, observ_t o)
{
    (void)c;
    (void)o;
    if (c != this && !o) {
        disconnect(c);
        return true;
    }
    return false;
}

bool QSimpleMessage::unsubscribe(const QObject *c)
{
    if (c) disconnect(c);
    return external_ && !topic_.isEmpty()
            && receivers(SIGNAL(on_message(QMessageResultPointer))) == 0;
}

QtPromise::QPromise<QVector<QVariant> > QSimpleMessage::publish(QEventQueue *queue, const QVariant &msg)
{
    queue_ = queue;
    last_ = msg;
    int n = receivers(SIGNAL(on_message(QMessageResultPointer)));
    if (n == 0)
        return QtPromise::QPromise<QVector<QVariant>>::resolve({});
    QMessageResultPointer data(msg);
    auto pm = data.await<QVariant, QVariant>(n);
    emit on_message(data);
    return pm;
}

void QSimpleMessage::mergeTo(QMessageBase *msg)
{
    // TODO:
    (void) msg;
    assert(false);
}
