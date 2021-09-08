#ifndef QMESSAGE_H
#define QMESSAGE_H

#include "QtEventBus_global.h"
#include "qmessageresult.h"

#include <QObject>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QSharedDataPointer>
#include <QVariant>

#include <functional>

class QEventQueue;

class QTEVENTBUS_EXPORT QMessageBase : public QObject
{
    Q_OBJECT

public:
    QMessageBase(bool external, bool stick, QByteArray const & topic);

public:
    QByteArray const & topic() const;

    QEventQueue * queue() const;

signals:
    void on_message(QMessageResultPointer data);

    void on_stick_message(QMessageResultPointer data);

public:
    /* stringlize */
    typedef std::function<QVariant (QByteArray const &, QVariant const &)> observ_t;

    bool subscribe(observ_t o, bool recv_stick) {
        return subscribe(this, o, recv_stick);
    }

    bool unsubscribe(observ_t o) {
        (void)o;
        return unsubscribe(this, o);
    }

    virtual bool subscribe(QObject const * c, observ_t o, bool recv_stick) = 0;

    virtual bool unsubscribe(QObject const * c, observ_t o) = 0;

    virtual bool unsubscribe(QObject const * c) = 0;

    virtual QtPromise::QPromise<QVector<QVariant>> publish(QVariant const & msg);

    virtual QtPromise::QPromise<QVector<QVariant>> publish(QEventQueue * queue, QVariant const & msg) = 0;

protected:
    bool external_;
    bool stick_;
    QByteArray const topic_;
    QEventQueue * queue_;
};

class QSimpleMessage : public QMessageBase
{
public:
    QSimpleMessage(QByteArray const & topic = nullptr)
        : QMessageBase(false, false, topic)
    {
    }

    virtual bool subscribe(QObject const * c, observ_t o, bool recv_stick);

    virtual bool unsubscribe(QObject const * c, observ_t o);

    virtual bool unsubscribe(QObject const * c);

    // from queue
    virtual QtPromise::QPromise<QVector<QVariant>> publish(QEventQueue * queue, QVariant const & msg);

private:
    QVariant last_;
};


template<typename T>
class QMessage : public QMessageBase
{
    typedef typename QMessageMeta<T>::result R;
public:
    QMessage()
        : QMessageBase(QMessageMeta<T>::external, QMessageMeta<T>::stick,
                       QMessageMeta<T>::topic ? QMessageMeta<T>::topic : "")
    {
    }

    ~QMessage() {
        if (last_)
            last_->~T();
    }

public:
    // register receiver f on this context
    template<typename F>
    bool subscribe(F f, bool recv_stick) {
        return subscribe(this, f, recv_stick);
    }

    // remove receiver f on this context
    template<typename F>
    bool unsubscribe(F f) {
        return unsubscribe(this, f);
    }

    // register receiver f on context o, return true if no receivers before
    template<typename F>
    bool subscribe(QObject const * c, F f, bool recv_stick) {
        QObject::connect(this, &QMessageBase::on_message, c,
                         [f](QMessageResultPointer const & e){e.invoke<T, R>(f);});
        if (last_ && recv_stick) {
            // send stick message on specific context
            auto cn = QObject::connect(this, &QMessageBase::on_stick_message, c,
                             [f](QMessageResultPointer const & e){e.invoke2<T, R>(f);});
            emit on_stick_message(*last_);
            disconnect(c, SIGNAL(on_stick_message(QMessageResultPointer)));
        }
        return external_ && !topic_.isEmpty()
                && receivers(SIGNAL(on_message(QMessageResultPointer))) == 1;
    }

    // remove receiver f on context o
    template<typename F>
    bool unsubscribe(QObject const * c, F f) {
        (void)c;
        (void)f;
        return false;
    }

    // remove all receivers on context o, return true if no receivers remain
    virtual bool unsubscribe(QObject const * c) override {
        QObject::disconnect(this, &QMessageBase::on_message, c, 0);
        return external_ && !topic_.isEmpty()
                && receivers(SIGNAL(on_message(QMessageResultPointer))) == 0;
    }

    QtPromise::QPromise<QVector<R>> publish(T const & msg) {
        return publish(nullptr, msg);
    }

    // publish msg, and may save for stick
    QtPromise::QPromise<QVector<R>> publish(QEventQueue * queue, T const & msg) {
        if (stick_) {
            if (last_)
                last_->~T();
            else
                last_ = reinterpret_cast<T *>(last_buf_);
            new (last_) T(msg);
        }
        queue_ = queue;
        int n = receivers(SIGNAL(on_message(QMessageResultPointer)));
        if (n == 0)
            return QtPromise::QPromise<QVector<R>>::resolve({});
        QMessageResultPointer data(msg);
        auto pm = data.await<T, R>(n);
        emit on_message(std::move(data));
        return pm;
    }

public:
    /* stringlize */

    virtual bool subscribe(QObject const * c, observ_t o, bool recv_stick) {
        return subscribe(c, [=](auto & m) { return fromVarR(o(topic_, toVar(m))); }, recv_stick);
    }

    virtual bool unsubscribe(QObject const * c, observ_t o) {
        (void)c;
        (void)o;
        return false;
    }

    virtual QtPromise::QPromise<QVector<QVariant>> publish(QEventQueue * queue, QVariant const & msg) {
        return publish(queue, fromVar(msg)).map([](R const & r, int) { return toVarR(r); });
    }

    template<typename F>
    bool subscribe2(F f, bool recv_stick) {
        return subscribe(this, [=](auto & m) { return f(topic_, toVar(m)); }, recv_stick);
    }

    template<typename F>
    bool unsubscribe2(F f) {
        return unsubscribe(this, [=](auto & m) { return f(topic_, toVar(m)); });
    }

    template<typename F>
    bool subscribe2(QObject const * c, F f, bool recv_stick) {
        return subscribe(c, [=](auto & m) { return f(topic_, toVar(m)); }, recv_stick);
    }

    template<typename F>
    bool unsubscribe2(QObject const * c, F f) {
        return unsubscribe(c, [=](auto & m) { return f(topic_, toVar(m)); });
    }

private:
    static QVariant toVar(T const & msg) {
        return QVariant::fromValue(msg);
    }

    static T fromVar(QVariant const & msg) {
        return msg.value<T>();
    }

    static QVariant toVarR(R const & result) {
        return QVariant::fromValue(result);
    }

    static R fromVarR(QVariant result) {
        return result.value<R>();
    }

private:
    T * last_ = nullptr;
    char last_buf_[sizeof(T)];
};

#endif // QMESSAGE_H
