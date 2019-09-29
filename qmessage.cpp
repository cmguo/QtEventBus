#include "qmessage.h"

QMessageBase::QMessageBase(bool external, bool stick, std::string const & topic)
    : external_(external)
    , stick_(stick)
    , topic_(topic)
{

}

std::string const & QMessageBase::topic() const {
    return topic_;
}
