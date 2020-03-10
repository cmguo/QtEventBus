#include "qeventbus.h"
#include "qsubscriber.h"

#include <QMetaMethod>
#include <QDebug>

QSubscriber::QSubscriber(QObject *receiver, const QByteArray &target)
{
    observ_ = [receiver, target](QByteArray const &, QVariant const & msg) {
        QSubscriber::invoke(receiver, target, msg);
    };
}

void QSubscriber::invoke(QObject *receiver, const QByteArray &target, QVariant args)
{
    QMetaObject const & meta = *receiver->metaObject();
    int index = meta.indexOfSlot(target);
    if (index < 0) {
        receiver->setProperty(target, args);
        return;
    }
    QMetaMethod method = meta.method(index);
    if (method.parameterCount() >= 4)
        return;
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
            return;
        if (!varg[i].convert(t))
            return;
        argv[i] = QGenericArgument(QMetaType::typeName(t), varg[i].data());
    }
    method.invoke(receiver, argv[0], argv[1], argv[2], argv[3]);
}

class TestReceiver : public QObject
{
public:
    void test(int i, QString s)
    {
        qDebug() << "TestReceiver" << i << s;
    }
};

void QSubscriber::test()
{
    QEventBus bus;
    TestReceiver receiver;
    bus.subscribe("test_topic", QSubscriber(&receiver, &TestReceiver::test));
    bus.publish("test_topic", QStringList({"1", "2"}));
}

void QSubscriber::operator()(const QByteArray &topic, const QVariant &msg)
{
    observ_(topic, msg);
}

