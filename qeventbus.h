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

    QEventBus(QEventBus * parent);

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
    QtPromise::QPromise<typename VectorResult<typename QMessageMeta<T>::result>::type>
    publish(T const & msg) {
        return get<T>().publish(msg);
    }

    // from queue
    template<typename T>
    QtPromise::QPromise<typename VectorResult<typename QMessageMeta<T>::result>::type>
    publish(QEventQueue * queue, T const & msg) {
        return get<T>().publish(queue, msg);
    }

public:
    void subscribe(QByteArray const & topic, QMessageBase::observ_t const & o, bool recv_stick = false) {
        if (get(topic).subscribe(o, recv_stick)) {
            for (QEventQueue * q : queues_)
                q->subscribe(topic);
        }
    }

    void unsubscribe(QByteArray const & topic, QMessageBase::observ_t const & o) {
        if (get(topic).unsubscribe(o)) {
            for (QEventQueue * q : queues_)
                q->unsubscribe(topic);
        }
    }

    void subscribe(QObject const * c, QByteArray const & topic, QMessageBase::observ_t const & o, bool recv_stick = false) {
        if (get(topic).subscribe(c, o, recv_stick)) {
            for (QEventQueue * q : queues_)
                q->subscribe(topic);
        }
    }

    void unsubscribe(QObject const * c, QByteArray const & topic, QMessageBase::observ_t const & o = nullptr) {
        if (get(topic).unsubscribe(c, o)) {
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
        QMessage<T> * msg = const_cast<QEventBus const *>(this)->get<T>();
        if (msg == nullptr) {
            msg = new QMessage<T>();
            void(*id)() = &event_id<T>;
            messages_.insert(std::make_pair(id, msg)).first;
            if (!msg->topic().isEmpty()) {
                auto rt = topics_.insert(std::make_pair(msg->topic(), msg));
                if (!rt.second) {
                    QSimpleMessage * old = static_cast<QSimpleMessage*>(rt.first->second);
                    old->mergeTo(msg);
                    rt.first->second = msg;
                    delete old;
                }
            }
        }
        return *msg;
    }

    template<typename T>
    QMessage<T> * get() const {
        void(*id)() = &event_id<T>;
        auto it = messages_.find(id);
        if (it == messages_.end()) {
            QEventBus const * parent = qobject_cast<QEventBus *>(this->parent());
            if (parent) {
                return parent->get<T>();
            } else {
                return nullptr;
            }
        }
        return static_cast<QMessage<T> *>(it->second);
    }

    QMessageBase & get(QByteArray const & topic);

    QMessageBase * get(QByteArray const & topic) const;

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
