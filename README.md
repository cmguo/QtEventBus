# QtEventBus
A event bus for Qt.

# 特性：
+ 订阅、取消订阅、发布消息
+ 通过消息的类型（class）区分消息
+ 可以指定接收线程（QObject绑定）
+ 可以绑定主题，通过主题订阅和接收（字符串消息）
+ 可以通过任意类型（QVaraint）发布、处理消息，可应用于和脚本语言之间的消息传递
+ 可以异步返回处理结果（来源于订阅者）给发布者（感谢 [QtPromise](https://github.com/simonbrunel/qtpromise))
+ 可以对接外部消息转发系统（通过主题对应，字符串消息）
+ 可以接收过去最后一个消息（粘性消息，订阅时单独发送）

# 原理：
尽量利用 Qt 的机制，利用 signal 机制，可以获得以下能力：
+ 与QObject绑定，订阅者一般是 QObject（信号连接需要一个接收的QObject）
+ 异步 publish，订阅者可以自定义接收线程（QObject的驻扎线程）
+ 生命期安全，当接收 QObject 销毁时，异步消息会自动清空
+ 生命期安全，当接收 QObject 销毁时，信号连接（订阅）会自动删除

# 使用：
+ 创建一个消息类
```cpp
class QTestMessage
{
public:
    std::string name;
    QTestMessage() {
        name = "test";
    }
    QTestMessage(QTestMessage const & o) {
        name = o.name;
    }
    ~QTestMessage() {
    }
};
// 消息模式（external, stick, topic）
Q_MESSAGE_META(QTestMessage, false, true, nullptr)
```
+ 订阅、发布消息
```cpp
QEventBus bus; // or QEventBus::globalInstance()
bus.subscribe<QTestMessage>(test, [](auto & m) {
    qDebug() << m.name.c_str();
});
bus.publish(QTestMessage());
```

+ 与脚本语言交互
``` js
bus.publish("test", {"name": "xxx"})
```
只需要在 C++ 里面注册 Variant 转换
``` cpp
QMetaType::registerConverter<QTestMessage, QJsonObject>([](auto & m) {
    ...
});
```

+ 异步等待结果
``` cpp
// declare result type
struct TestMessage
{
    int i;
    int j;
};
Q_MESSAGE_META_RESULT(TestMessage, false, false, "test", int)

bus.subscribe<TestMessage>([] (auto msg) {
    return QtPromise::resolve(msg.i + msg.j);
});

bus.publish(TestMessage{1, 2}).then([](QVector<int> const & result) {
    qDebug() << "TestMessage public result:" << result;
}, [](std::exception & e) {
    qDebug() << e.what();
});
```
