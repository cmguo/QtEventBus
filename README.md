# QEventBus
A event bus for Qt.

# 特性：
+ 订阅、取消订阅、发布消息
+ 通过消息的类型（class）区分消息
+ 可以指定接收线程（QObject绑定）
+ 可以绑定主题，通过主题订阅和接收（字符串消息）
+ 可以对接外部消息转发系统（通过主题对应，字符串消息）
+ 可以接收过去最后一个消息（粘性消息，订阅时单独发送）

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
    QEventBus bus;
    bus.publish(QTestMessage());
    bus.subscribe<QTestMessage>(test, [](auto & m) {
        qDebug() << m.name.c_str();
    });
```
