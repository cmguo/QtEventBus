#include "qsubscriber.h"

QSubscriber::QSubscriber(QObject *receiver, const QByteArray &method)
{
    observ_ = [](QByteArray const &, QVariant const & msg) {

    };
}
