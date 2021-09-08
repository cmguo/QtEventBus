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
    auto it = topics_.find(topic);
    if (it == topics_.end()) {
       it = topics_.insert(std::make_pair(topic, new QSimpleMessage(topic))).first;
    }
    return *it->second;
}
