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
        msg_ = o.msg_ = o.copy_(o.msg_);
        o.ref_(msg_, true);
        o.copy_ = nullptr;
    } else {
        // can't go here
        msg_ = o.msg_;
    }
    copy_ = nullptr;
    ref_ = o.ref_;
    ref_(msg_, true);
}

QMessageData::~QMessageData()
{
    if (!copy_ && ref_)
        ref_(msg_, false);
}

