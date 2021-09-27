#ifndef QTESTMESSAGE_H
#define QTESTMESSAGE_H

#include <string>
#include "qmessage.h"

class QTestMessage
{
public:
    std::string name;

    QTestMessage() {
        name = "test";
    }

    QTestMessage(QTestMessage const & o) {
        name = o.name;
    }

    ~QTestMessage() {

    }
};

Q_MESSAGE_META(QTestMessage, false, true, nullptr)

#endif // QTESTMESSAGE_H
