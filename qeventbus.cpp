#include "qeventbus.h"

#include "qexport.h"

#include <qcomponentcontainer.h>

static QExport<QEventBus> export_eventbus(QPart::shared);

Q_DECLARE_METATYPE(QMessageData)

QEventBus &QEventBus::globalInstance()
{
    return *QComponentContainer::globalInstance().getExportValue<QEventBus>();
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
