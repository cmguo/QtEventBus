#include "qeventbus.h"

#include "qeventbusqml.h"
#include "qexport.h"

#include <qcomponentcontainer.h>
#include <QQmlEngine>


static QExport<QEventBus> export_eventbus(QPart::shared);

Q_DECLARE_METATYPE(QMessageResultPointer)

QEventBus &QEventBus::globalInstance()
{
    return *QComponentContainer::globalInstance().getExportValue<QEventBus>();
}

void QEventBus::init()
{
    static bool initialized = false;
    if (initialized) {
        return;
    }

    qmlRegisterSingletonType<QEventBusQml>("QEventBus", 1, 0, "QEventBus", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        QEventBusQml *obj = new QEventBusQml();
        return obj;
    });

    initialized = true;
}

QEventBus::QEventBus()
{
    qRegisterMetaType<QMessageResultPointer>();
}

QEventBus::QEventBus(QEventBus *parent)
    : QObject(parent)
{
}

void QEventBus::onComposition()
{
    for (QEventQueue * q : queues_) {
        connect(q, &QEventQueue::onMessage, this, &QEventBus::onMessage);
    }
}

void QEventBus::onMessage(const QByteArray &topic, const QVariant &msg)
{
    publish(qobject_cast<QEventQueue*>(sender()), topic, msg);
}

QtPromise::QPromise<QVector<QVariant>> QEventBus::publish(const QByteArray &topic, const QVariant &msg)
{
    return get(topic).publish(msg);
    // TODO: also publish to queues
}

QtPromise::QPromise<QVector<QVariant>> QEventBus::publish(QEventQueue *queue, const QByteArray &topic, const QVariant &msg)
{
    return get(topic).publish(queue, msg);
}

QMessageBase &QEventBus::get(const QByteArray &topic)
{
    QMessageBase * msg = const_cast<QEventBus const *>(this)->get(topic);
    if (msg == nullptr) {
        msg = new QSimpleMessage(topic);
        topics_.insert(std::make_pair(topic, msg));
    }
    return *msg;
}

QMessageBase * QEventBus::get(const QByteArray &topic) const
{
    auto it = topics_.find(topic);
    if (it == topics_.end()) {
        QEventBus const * parent = qobject_cast<QEventBus *>(this->parent());
        if (parent) {
            return parent->get(topic);
        }
        return nullptr;
    }
    return it->second;
}

void QEventBus::put(const QByteArray &topic, QMessageBase *msg)
{
    auto rt = topics_.insert(std::make_pair(topic, msg));
    if (!rt.second) {
        QSimpleMessage * old = static_cast<QSimpleMessage*>(rt.first->second);
        old->mergeTo(msg);
        rt.first->second = msg;
        delete old;
    }
}
