#ifndef QEVENTBUS_H
#define QEVENTBUS_H

#include "QtEventBus_global.h"
#include "qmessage.h"
#include "qeventqueue.h"

#include <functional>
#include <vector>
#include <map>

class QTEVENTBUS_EXPORT QEventBus : public QObject
{
    Q_OBJECT

public:
    static QEventBus & globalInstance();

    static void init();

public:
    Q_INVOKABLE QEventBus();

public:
    template<typename T, typename F>
    void subscribe(F f, bool recv_stick = false) {
        if (get<T>().subscribe(f, recv_stick) && queue_)
            queue_->subscribe(get<T>().topic());
    }

    template<typename T, typename F>
    void unsubscribe(F f) {
        if (get<T>().unsubscribe(f) && queue_)
            queue_->unsubscribe(get<T>().topic());
    }

    template<typename T, typename F>
    void subscribe(QObject const * o, F f, bool recv_stick = false) {
        if (get<T>().subscribe(o, f, recv_stick) && queue_)
            queue_->subscribe(get<T>().topic());
    }

    template<typename T, typename F>
    void unsubscribe(QObject const * o, F f) {
        if (get<T>().unsubscribe(o, f) && queue_)
            queue_->unsubscribe(get<T>().topic());
    }

    template<typename T>
    void unsubscribe(QObject const * o) {
        if (get<T>().unsubscribe(o) && queue_)
            queue_->unsubscribe(get<T>().topic());
    }

    template<typename T>
    void publish(T const & msg) {
        get<T>().publish(msg);
    }

public:
    template<typename F>
    void subscribe(QByteArray const & topic, F f, bool recv_stick = false) {
        get(topic).subscribe(f, recv_stick);
    }

    template<typename F>
    void unsubscribe(QByteArray const & topic, F f) {
        get(topic).unsubscribe(f);
    }

    template<typename F>
    void subscribe(QObject const * c, QByteArray const & topic, F f, bool recv_stick = false) {
        get(topic).subscribe(c, f, recv_stick);
    }

    template<typename F>
    void unsubscribe(QObject const * c, QByteArray const & topic, F f) {
        get(topic).unsubscribe(c, f);
    }

    void publish(QByteArray const & topic, QVariant const & msg = QVariant());

private:
    template<typename T>
    QMessage<T> & get() {
        void(*id)() = &event_id<T>;
        auto it = messages_.find(id);
        if (it == messages_.end()) {
            QMessage<T> * msg = new QMessage<T>();
            it = messages_.insert(std::make_pair(id, msg)).first;
            if (!msg->topic().empty())
                topics_.insert(std::make_pair(msg->topic(), msg));
        }
        return static_cast<QMessage<T> &>(*it->second);
    }

    QMessageBase & get(QByteArray const & topic);

private:
    template<typename T>
    static void event_id() {}

    QEventQueue * queue_;
    // messages with type (event_id)
    std::map<void(*)(), QMessageBase *> messages_;
    // messages with external topic or without type
    std::map<QByteArray, QMessageBase *> topics_;
};

#endif // QEVENTBUS_H
