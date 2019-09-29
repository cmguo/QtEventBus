#ifndef QEVENTBUS_H
#define QEVENTBUS_H

#include "QtEventBus_global.h"
#include "qmessage.h"
#include "qeventqueue.h"

#include <functional>
#include <vector>
#include <map>

class QTEVENTBUS_EXPORT QEventBus : QObject
{
    Q_OBJECT

public:
    QEventBus();

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
    void subscribe(std::string const & topic, F f, bool recv_stick = false) {
        auto it = topics_.find(topic);
        if (it != topics_.end()) {
            it->second->subscribe(f, recv_stick);
        }
    }

    template<typename F>
    void unsubscribe(std::string const & topic, F f) {
        auto it = topics_.find(topic);
        if (it != topics_.end()) {
            it->second->unsubscribe(f);
        }
    }

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

private:
    template<typename T>
    static void event_id() {}

    QEventQueue * queue_;
    std::map<void(*)(), QMessageBase *> messages_;
    std::map<std::string, QMessageBase *> topics_;
};

#endif // QEVENTBUS_H
