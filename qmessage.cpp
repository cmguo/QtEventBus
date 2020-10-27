#include "qmessage.h"

QMessageBase::QMessageBase(bool external, bool stick, QByteArray const & topic)
    : external_(external)
    , stick_(stick)
    , topic_(topic)
{

}

QByteArray const & QMessageBase::topic() const
{
    return topic_;
}

QEventQueue *QMessageBase::queue() const
{
    return queue_;
}

void QMessageBase::publish(const QVariant &msg)
{
    publish(nullptr, msg);
}
