#ifndef QSUBSCRIBER_H
#define QSUBSCRIBER_H

#include <QtEventBus_global.h>

#include "qmessage.h"

#include <QByteArray>
#include <QVariant>
#include <QVector>

class QTEVENTBUS_EXPORT QSubscriber
{
public:
    QSubscriber(QObject * receiver, QByteArray const & target);


    template <typename Func>
    QSubscriber(typename QtPrivate::FunctionPointer<Func>::Object *receiver, Func func)
    {
        observ_ = [receiver, func](QByteArray const &, QVariant const & msg) {
            return QSubscriber::invoke(receiver, func, msg);
        };
    }

public:
    static void test();

public:
    QVariant operator()(QByteArray const & topic, QVariant const & msg) const;

private:
    static QVariant invoke(QObject * receiver, QByteArray const & target, QVariant args);

    template <typename Func>
    static QVariant invoke(typename QtPrivate::FunctionPointer<Func>::Object * receiver, Func func, QVariant args)
    {
        typedef QtPrivate::FunctionPointer<Func> FuncType;
        QVariantList list;
        if (args.canConvert(qMetaTypeId<QVariantList>())
            && args.convert(qMetaTypeId<QVariantList>())) {
            list = args.toList();
        } else {
            list.append(args);
        }
        typename FuncType::Arguments types;
        fillArg<int>(types, 0, list);
        QVector<void*> vec;
        QVariant ret;
        vec.append(ret.data());
        for (QVariant & l : list)
            vec.append(l.data());
        FuncType::template call<typename FuncType::Arguments, QVariant>(func, receiver, vec.data());
        return ret;
    }

    template<typename U, typename Arg, typename ...Args>
    static bool fillArg(QtPrivate::List<Arg, Args...>, int index, QVariantList & list)
    {
        if (index >= list.size())
            list.append(QVariant());
        QVariant & arg = list[index];
        int t = qMetaTypeId<Arg>();
        if (!arg.canConvert(t))
            return false;
        if (!arg.convert(t))
            return false;
        return fillArg<U>(QtPrivate::List<Args...>(), ++index, list);
    }

    template<typename U>
    static bool fillArg(QtPrivate::List<>, int, QVariantList &)
    {
        return true;
    }

private:
    QMessageBase::observ_t observ_;
};

#endif // QSUBSCRIBER_H
