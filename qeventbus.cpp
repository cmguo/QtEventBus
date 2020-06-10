#include "qeventbus.h"

#include "qeventbusqml.h"
#include "qexport.h"

#include <qcomponentcontainer.h>
#include <QQmlEngine>


static QExport<QEventBus> export_eventbus(QPart::shared);

Q_DECLARE_METATYPE(QMessageData)

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
    qRegisterMetaType<QMessageData>();
}

void QEventBus::publish(const QByteArray &topic, const QVariant &msg)
{
    get(topic).publish(msg);
}

QMessageBase &QEventBus::get(const QByteArray &topic)
{
    auto it = topics_.find(topic);
    if (it == topics_.end()) {
       it = topics_.insert(std::make_pair(topic, new QSimpleMessage(topic))).first;
    }
    return *it->second;
}
