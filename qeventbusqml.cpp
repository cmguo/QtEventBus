#include "qeventbus.h"
#include "qeventbusqml.h"
#include <QVariant>

QEventBusQml::QEventBusQml(QObject *parent) : QObject(parent)
{

}

void QEventBusQml::publish(QString what, QVariantMap value)
{
    QEventBus::globalInstance().publish(what.toUtf8(),value);
}
