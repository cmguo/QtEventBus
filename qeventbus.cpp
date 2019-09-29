#include "qeventbus.h"

#include "qexport.h"

static QExport<QEventBus> export_eventbus;

Q_DECLARE_METATYPE(QMessageData)

QEventBus::QEventBus()
{
    qRegisterMetaType<QMessageData>();
}
