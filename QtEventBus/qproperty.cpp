#include "qproperty.h"

#include <QMetaProperty>
#include <QDebug>

QVariant QProperty::getProp(QVariant const & obj, char const * prop)
{
    if (prop != nullptr && prop[0] == 0)
        return obj;
    QMetaObject const * meta = getMeta(obj);
    if (meta) {
        int index = meta->indexOfProperty(prop);
        if (index >= 0) {
            QMetaProperty p = meta->property(index);
            if (meta->inherits(&QObject::staticMetaObject))
                return p.read(obj.value<QObject *>());
            else if (QMetaType::typeFlags(obj.userType()).testFlag(QMetaType::PointerToGadget))
                return p.readOnGadget(*reinterpret_cast<void const * const *>(obj.constData()));
            else
                return p.readOnGadget(obj.constData());
        }
    }
    return QVariant();
}

void QProperty::setProp(QVariant & obj, char const * prop, QVariant && value)
{
    QMetaObject const * meta = getMeta(obj);
    if (meta) {
        int index = meta->indexOfProperty(prop);
        if (index >= 0) {
            QMetaProperty p = meta->property(index);
            if (meta->inherits(&QObject::staticMetaObject))
                p.write(obj.value<QObject *>(), value);
            else if (QMetaType::typeFlags(obj.userType()).testFlag(QMetaType::PointerToGadget))
                p.writeOnGadget(*reinterpret_cast<void * const *>(obj.data()), value);
            else
                p.writeOnGadget(obj.data(), value);
        }
    }
}

QMetaObject const * QProperty::getMeta(QVariant const & obj)
{
    if (obj.type() == QVariant::UserType) {
        if (obj.userType() == QMetaType::QObjectStar)
            return obj.value<QObject *>()->metaObject();
        else
            return QMetaType::metaObjectForType(obj.userType());
    }
    return nullptr;
}

void QProperty::dumpProps(const QVariant &obj)
{
    QMetaObject const * meta = getMeta(obj);
    if (meta) {
        for (int index = 0; index < meta->propertyCount(); ++index) {
            QMetaProperty p = meta->property(index);
            if (meta->inherits(&QObject::staticMetaObject))
                qDebug() << p.name() << ":" << p.read(obj.value<QObject *>());
            else if (QMetaType::typeFlags(obj.userType()).testFlag(QMetaType::PointerToGadget))
                qDebug() << p.name() << ":" << p.readOnGadget(*reinterpret_cast<void const * const *>(obj.constData()));
            else
                qDebug() << p.name() << ":" << p.readOnGadget(obj.constData());
        }
    }
}
