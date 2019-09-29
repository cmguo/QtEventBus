#ifndef QMESSAGEPUSH_H
#define QMESSAGEPUSH_H

#include <QObject>
#include <string>

class QEventQueue : QObject
{
public:
    virtual void subscribe(std::string const & topic) = 0;

    virtual void unsubscribe(std::string const & topic) = 0;

    virtual void publish(std::string const & topic, std::string const & msg) = 0;
};

#endif // QMESSAGEPUSH_H
