#ifndef QMESSAGE_H
#define QMESSAGE_H

#include "QtEventBus_global.h"
#include "qmessagedata.h"

#include <QObject>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QSharedDataPointer>

#include <functional>
#include <vector>
#include <string>

class QTEVENTBUS_EXPORT QMessageBase : public QObject
{
    Q_OBJECT

public:
    QMessageBase(bool external, bool stick, std::string const & topic);

public:
    std::string const & topic() const;

signals:
    void on_message(QMessageData const & msg);

    void on_stick_message(QMessageData const & msg);

public:
    /* stringlize */
    typedef std::function<void(std::string const &, std::string const &)> observ_t;

    virtual void subscribe(observ_t o, bool recv_stick) = 0;

    virtual void unsubscribe(observ_t o) = 0;

    virtual void publish(std::string const & msg) = 0;

protected:
    bool external_;
    bool stick_;
    std::string const topic_;
};

template<typename T>
class QMessageMeta
{
public:
    static constexpr bool external = false;
    static constexpr bool stick = false;
    static constexpr char const * topic = nullptr;
};

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
    template<typename F>
    bool subscribe(F f, bool recv_stick) {
        return subscribe(this, f, recv_stick);
    }

    template<typename F>
    bool unsubscribe(F f) {
        return unsubscribe(this, f);
    }

    template<typename F>
    bool subscribe(QObject const * o, F f, bool recv_stick) {
        QObject::connect(this, &QMessageBase::on_message, o,
                         [f](QMessageData const & e){f(e.get<T>());});
        if (last_ && recv_stick) {
            auto c = QObject::connect(this, &QMessageBase::on_stick_message, o,
                             [f](QMessageData const & e){f(e.get<T>());});
            emit on_stick_message(*last_);
            QObject::disconnect(c);
        }
        return external_ && !topic_.empty()
                && receivers(SIGNAL(on_message(QMessageData const &))) == 1;
    }

    template<typename F>
    bool unsubscribe(QObject const * o, F f) {
        (void)o;
        (void)f;
        return false;
    }

    bool unsubscribe(QObject const * o) {
        QObject::disconnect(this, &QMessageBase::on_message, o, 0);
        return external_ && !topic_.empty()
                && receivers(SIGNAL(on_message(QMessageData const &))) == 0;
    }

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

    virtual void subscribe(observ_t o, bool recv_stick) {
        subscribe(this, [=](auto m){o(topic_, to_json(m));}, recv_stick);
    }

    virtual void unsubscribe(observ_t o) {
        (void)o;
        unsubscribe(this, [=](auto m){o(topic_, to_json(m));});
    }

    virtual void publish(std::string const & msg) {
        publish(from_json(msg));
    }

    template<typename F>
    void subscribe2(F f) {
        subscribe(this, [=](auto m){f(topic_, to_json(m));});
    }

    template<typename F>
    void unsubscribe2(F f) {
        unsubscribe(this, [=](auto m){f(topic_, to_json(m));});
    }

private:
    std::string to_json(T const & msg) {
        (void)msg;
        return std::string();
    }

    T from_json(std::string const & msg) {
        (void)msg;
        return T();
    }

private:
    T * last_ = nullptr;
    char last_buf_[sizeof(T)];
};

#endif // QMESSAGE_H
