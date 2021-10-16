#include <qeventbus.h>
#include <qsubscriber.h>

class TestReceiver : public QObject
{
public:
    QString test(int i, QString s)
    {
        qDebug() << "TestReceiver" << i << s;
        return s + i;
    }
};

struct TestMessage
{
    int i;
    int j;
};

Q_MESSAGE_META_RESULT(TestMessage, false, false, "test", int)

struct VoidMessage
{
    int i;
    int j;
};

Q_MESSAGE_META(VoidMessage, false, false, "void")

void test()
{
    QEventBus bus;
    // SimpleMessage
    TestReceiver receiver;
    bus.subscribe("test_topic", QSubscriber(&receiver, &TestReceiver::test));
    bus.publish("test_topic", QStringList({"1", "2"})).then([](QVector<QVariant> const & result) {
        qDebug() << "SimpleMessage public result:" << result;
    }, [](std::exception & e) {
        qDebug() << e.what();
    });
    // TestMessage (test)
    QMetaType::registerConverter<TestMessage, QVariantList>([](auto & m) {
        return QVariantList{m.i, m.j};
    });
    QMetaType::registerConverter<QVariantList, TestMessage>([](auto & l) {
        return TestMessage{l[0].toInt(), l[1].toInt()};
    });
    bus.subscribe<TestMessage>([] (auto msg) {
        return QtPromise::resolve(msg.i + msg.j);
    });
    bus.subscribe("test", QSubscriber(&receiver, &TestReceiver::test));
    bus.subscribe("test", [](auto topic, auto message) {
        qDebug() << "test" << topic << message.toList();
    });
    bus.publish(TestMessage{1, 2}).then([](QVector<int> const & result) {
        qDebug() << "TestMessage public result:" << result;
    }, [](std::exception & e) {
        qDebug() << e.what();
    });
    bus.publish("test", QVariantList{1, 2}).then([](QVector<QVariant> const & result) {
        qDebug() << "TestMessage variant public result:" << result;
    }, [](std::exception & e) {
        qDebug() << e.what();
    });;
    // VoidMessage (void)
    QMetaType::registerConverter<VoidMessage, QVariantList>([](auto & m) {
        return QVariantList{m.i, m.j};
    });
    QMetaType::registerConverter<QVariantList, VoidMessage>([](auto & l) {
        return VoidMessage{l[0].toInt(), l[1].toInt()};
    });
    bus.subscribe<VoidMessage>([] (auto) {
        return QtPromise::resolve();
    });
    bus.subscribe("void", QSubscriber(&receiver, &TestReceiver::test));
    bus.publish(VoidMessage{1, 2}).then([]() {
        qDebug() << "VoidMessage public result";
    }, [](std::exception & e) {
        qDebug() << e.what();
    });
    bus.publish("void", QVariantList{1, 2}).then([](QVector<QVariant> const & result) {
        qDebug() << "VoidMessage variant public result:" << result;
    }, [](std::exception & e) {
        qDebug() << e.what();
    });
}
