#ifndef QPROPERTY_H
#define QPROPERTY_H

#include "QtEventBus_global.h"

#include <QVariant>

class QTEVENTBUS_EXPORT QProperty
{
public:
    static QVariant getProp(QVariant const & obj, char const * prop);

    static void setProp(QVariant & obj, char const * prop, QVariant && value);

    static QMetaObject const * getMeta(QVariant const & obj);

    static void dumpProps(QVariant const & obj);
};

#endif // QPROPERTY_H
