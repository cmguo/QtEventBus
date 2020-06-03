#include "qmessagedata.h"

QMessageData::QMessageData()
    : msg_(nullptr)
    , copy_(nullptr)
    , ref_(nullptr)
{
}

QMessageData::QMessageData(QMessageData const & o)
{
    if (o.copy_) {
        // if <o> holds original msg instance, make a shared copy
        msg_ = o.msg_ = o.copy_(o.msg_);
        o.ref_(msg_, true);
        o.copy_ = nullptr;
    } else {
        // can't go here
        // if <o> holds shared copy, continue share
        msg_ = o.msg_;
    }
    // anyway, we hold shared copy now
    copy_ = nullptr;
    ref_ = o.ref_;
    ref_(msg_, true);
}

QMessageData::~QMessageData()
{
    // if we hold shared copy, unref it, we may empty (ref_ == null)
    if (!copy_ && ref_)
        ref_(msg_, false);
}

