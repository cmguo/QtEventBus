#ifndef QMESSAGEDATA_H
#define QMESSAGEDATA_H

#include "QtEventBus_global.h"

#include <QSharedData>

template<typename T>
class QMessageSharedData : public QSharedData
{
public:
    QMessageSharedData(T const & msg)
        : msg_(msg)
    {
    }

    T const msg_;
};

class QMessageData
{
public:
    QMessageData();

    template<typename T>
    QMessageData(T const & msg)
    {
        msg_ = &msg;
        copy_ = &copy<T>;
        ref_ = &ref<T>;
    }

    QMessageData(QMessageData const & o);

    QTEVENTBUS_EXPORT ~QMessageData();

public:
    template<typename T>
    T const & get() const
    {
        return *reinterpret_cast<T const *>(msg_);
    }

private:
    template<typename T>
    static void const * copy(void const * msg) {
        return &(new QMessageSharedData<T>(*reinterpret_cast<T const *>(msg)))->msg_;
    }

    template<typename T>
    static void ref(void const * msg, bool inc) {
        QMessageSharedData<T> const * sd =
                reinterpret_cast<QMessageSharedData<T> const *>(
                    reinterpret_cast<char const *>(msg) - offsetof(QMessageSharedData<T>, msg_));
        if (inc)
            sd->ref.ref();
        else if (!sd->ref.deref())
            delete sd;
    }

private:
    mutable void const * msg_;
    mutable void const * (* copy_)(void const *);
    void (*ref_)(void const *, bool);
};

#endif // QMESSAGEDATA_H
