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
        QMessage<T> & msg = get<T>();
        if (msg.subscribe(f, recv_stick)) {
            for (QEventQueue * q : queues_)
                q->subscribe(msg.topic());
        }
    }

    template<typename T, typename F>
    void unsubscribe(F f) {
        QMessage<T> & msg = get<T>();
        if (get<T>().unsubscribe(f)) {
            for (QEventQueue * q : queues_)
                q->unsubscribe(get<T>().topic());
        }
    }

    template<typename T, typename F>
    void subscribe(QObject const * o, F f, bool recv_stick = false) {
        QMessage<T> & msg = get<T>();
        if (msg.subscribe(o, f, recv_stick)) {
            for (QEventQueue * q : queues_)
                q->subscribe(msg.topic());
        }
    }

    template<typename T, typename F>
    void unsubscribe(QObject const * o, F f) {
        QMessageBase & msg = get<T>();
        if (msg.unsubscribe(o, f)) {
            for (QEventQueue * q : queues_)
                q->unsubscribe(msg.topic());
        }
    }

    template<typename T>
    void unsubscribe(QObject const * o) {
        QMessage<T> & msg = get<T>();
        if (msg.unsubscribe(o)) {
            for (QEventQueue * q : queues_)
                q->unsubscribe(msg.topic());
        }
    }

    template<typename T>
    QtPromise::QPromise<QVector<typename QMessageMeta<T>::result>>
    publish(T const & msg) {
        return get<T>().publish(msg);
    }

    // from queue
    template<typename T>
    QtPromise::QPromise<QVector<typename QMessageMeta<T>::result>>
    publish(QEventQueue * queue, T const & msg) {
        return get<T>().publish(queue, msg);
    }

public:
    template<typename F>
    void subscribe(QByteArray const & topic, F f, bool recv_stick = false) {
        if (get(topic).subscribe(f, recv_stick)) {
            for (QEventQueue * q : queues_)
                q->subscribe(topic);
        }
    }

    template<typename F>
    void unsubscribe(QByteArray const & topic, F f) {
        if (get(topic).unsubscribe(f)) {
            for (QEventQueue * q : queues_)
                q->unsubscribe(topic);
        }
    }

    template<typename F>
    void subscribe(QObject const * c, QByteArray const & topic, F f, bool recv_stick = false) {
        if (get(topic).subscribe(c, f, recv_stick)) {
            for (QEventQueue * q : queues_)
                q->subscribe(topic);
        }
    }

    template<typename F>
    void unsubscribe(QObject const * c, QByteArray const & topic, F f) {
        if (get(topic).unsubscribe(c, f)) {
            for (QEventQueue * q : queues_)
                q->unsubscribe(topic);
        }
    }

    QtPromise::QPromise<QVector<QVariant>> publish(QByteArray const & topic, QVariant const & msg = QVariant());

    // from queue
    QtPromise::QPromise<QVector<QVariant>> publish(QEventQueue * queue, QByteArray const & topic, QVariant const & msg = QVariant());

private slots:
    void onComposition();

private:
    void onMessage(QByteArray const & topic, QVariant const & msg);

    template<typename T>
    QMessage<T> & get() {
        void(*id)() = &event_id<T>;
        auto it = messages_.find(id);
        if (it == messages_.end()) {
            QMessage<T> * msg = new QMessage<T>();
            it = messages_.insert(std::make_pair(id, msg)).first;
            if (!msg->topic().isEmpty())
                topics_.insert(std::make_pair(msg->topic(), msg));
        }
        return static_cast<QMessage<T> &>(*it->second);
    }

    QMessageBase & get(QByteArray const & topic);

private:
    template<typename T>
    static void event_id() {}

    QList<QEventQueue *> queues_;
    // messages with type (event_id)
    std::map<void(*)(), QMessageBase *> messages_;
    // messages with external topic or without type
    std::map<QByteArray, QMessageBase *> topics_;
};

#endif // QEVENTBUS_H
