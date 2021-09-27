#include "qeventbus.h"
#include "qsubscriber.h"

#include <QMetaMethod>
#include <QDebug>

QSubscriber::QSubscriber(QObject *receiver, const QByteArray &target)
{
    observ_ = [receiver, target](QByteArray const &, QVariant const & msg) {
        return QSubscriber::invoke(receiver, target, msg);
    };
}

QVariant QSubscriber::operator()(const QByteArray &topic, const QVariant &msg) const
{
    return observ_(topic, msg);
}

QVariant QSubscriber::invoke(QObject *receiver, const QByteArray &target, QVariant args)
{
    QMetaObject const & meta = *receiver->metaObject();
    int index = meta.indexOfSlot(target);
    if (index < 0) {
        receiver->setProperty(target, args);
        return QVariant();
    }
    QMetaMethod method = meta.method(index);
    if (method.parameterCount() >= 4)
        return QVariant();
    QGenericArgument argv[4];
    QVariant varg[4];
    QVariantList list;
    if (args.canConvert(qMetaTypeId<QVariantList>())
        && args.convert(qMetaTypeId<QVariantList>())) {
        list = args.toList();
    } else {
        list.append(args);
    }
    for (int i = 0; i < method.parameterCount(); ++i) {
        if (i < list.size())
            varg[i] = list[i];
        int t = method.parameterType(i);
        if (!varg[i].canConvert(t))
            return QVariant();
        if (!varg[i].convert(t))
            return QVariant();
        argv[i] = QGenericArgument(QMetaType::typeName(t), varg[i].data());
    }
    QVariant result;
    QGenericReturnArgument rtarg(QMetaType::typeName(method.returnType()), result.data());
    method.invoke(receiver, rtarg, argv[0], argv[1], argv[2], argv[3]);
    return result;
}
