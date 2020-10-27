#ifndef QMESSAGE_H
#define QMESSAGE_H

#include "QtEventBus_global.h"
#include "qmessagedata.h"

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
    void on_message(QMessageData const & msg);

    void on_stick_message(QMessageData const & msg);

public:
    /* stringlize */
    typedef std::function<void(QByteArray const &, QVariant const &)> observ_t;

    void subscribe(observ_t o, bool recv_stick) {
        subscribe(this, o, recv_stick);
    }

    void unsubscribe(observ_t o) {
        (void)o;
        unsubscribe(this, o);
    }

    virtual void subscribe(QObject const * c, observ_t o, bool recv_stick) = 0;

    virtual void unsubscribe(QObject const * c, observ_t o) = 0;

    virtual void publish(QVariant const & msg);

    virtual void publish(QEventQueue * queue, QVariant const & msg) = 0;

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

    virtual void subscribe(QObject const * c, observ_t o, bool recv_stick)
    {
        QObject::connect(this, &QMessageBase::on_message, c,
                         [this, o](QMessageData const & e){o(topic_, e.get<QVariant>());});
        if (last_.isValid() && recv_stick) {
            // send stick message on specific context
            auto cn = QObject::connect(this, &QMessageBase::on_stick_message, c,
                             [this, o](QMessageData const & e){o(topic_, e.get<QVariant>());});
            emit on_stick_message(last_);
            QObject::disconnect(c);
        }
    }

    virtual void unsubscribe(QObject const * c, observ_t o)
    {
        (void)c;
        (void)o;
    }

    virtual void publish(QEventQueue * queue, QVariant const & msg)
    {
        queue_ = queue;
        last_ = msg;
        emit on_message(msg);
    }

private:
    QVariant last_;
};

template<typename T>
class QMessageMeta
{
public:
    static constexpr bool external = false;
    static constexpr bool stick = false;
    static constexpr char const * topic = nullptr;
};

// register properties of this message type

#define Q_MESSAGE_META(ty, et, sk, tp) \
    template<> \
    class QMessageMeta<ty> \
    { \
    public: \
        static constexpr bool external = et; \
        static constexpr bool stick = sk; \
        static constexpr char const * topic = tp; \
    };

template<typename T>
class QMessage : public QMessageBase
{
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
                         [f](QMessageData const & e){f(e.get<T>());});
        if (last_ && recv_stick) {
            // send stick message on specific context
            auto cn = QObject::connect(this, &QMessageBase::on_stick_message, c,
                             [f](QMessageData const & e){f(e.get<T>());});
            emit on_stick_message(*last_);
            QObject::disconnect(c);
        }
        return external_ && !topic_.isEmpty()
                && receivers(SIGNAL(on_message(QMessageData const &))) == 1;
    }

    // remove receiver f on context o
    template<typename F>
    bool unsubscribe(QObject const * c, F f) {
        (void)c;
        (void)f;
        return false;
    }

    // remove all receivers on context o, return true if no receivers remain
    bool unsubscribe(QObject const * c) {
        QObject::disconnect(this, &QMessageBase::on_message, c, 0);
        return external_ && !topic_.isEmpty()
                && receivers(SIGNAL(on_message(QMessageData const &))) == 0;
    }

    // publish msg, and may save for stick
    void publish(T const & msg) {
        if (stick_) {
            if (last_)
                last_->~T();
            else
                last_ = reinterpret_cast<T *>(last_buf_);
            new (last_) T(msg);
        }
        emit on_message(QMessageData(msg));
    }

public:
    /* stringlize */

    virtual void subscribe(QObject const * c, observ_t o, bool recv_stick) {
        subscribe(c, [=](auto m){o(topic_, toVar(m));}, recv_stick);
    }

    virtual void unsubscribe(QObject const * c, observ_t o) {
        (void)o;
        unsubscribe(c, [=](auto m){o(topic_, toVar(m));});
    }

    virtual void publish(QVariant const & msg) {
        publish(fromVar(msg));
    }

    template<typename F>
    void subscribe2(F f, bool recv_stick) {
        subscribe(this, [=](auto m){f(topic_, toVar(m));}, recv_stick);
    }

    template<typename F>
    void unsubscribe2(F f) {
        unsubscribe(this, [=](auto m){f(topic_, toVar(m));});
    }

    template<typename F>
    void subscribe2(QObject const * c, F f, bool recv_stick) {
        subscribe(c, [=](auto m){f(topic_, toVar(m));}, recv_stick);
    }

    template<typename F>
    void unsubscribe2(QObject const * c, F f) {
        unsubscribe(c, [=](auto m){f(topic_, toVar(m));});
    }

private:
    QVariant toVar(T const & msg) {
        (void)msg;
        return QVariant();
    }

    T fromVar(QVariant const & msg) {
        (void)msg;
        return T();
    }

private:
    T * last_ = nullptr;
    char last_buf_[sizeof(T)];
};

#endif // QMESSAGE_H
