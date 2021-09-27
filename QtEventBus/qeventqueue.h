#ifndef QMESSAGEPUSH_H
#define QMESSAGEPUSH_H

#include <QObject>
#include <string>

class QEventQueue : public QObject
{
    Q_OBJECT
public:
    virtual void subscribe(QByteArray const & topic) = 0;

    virtual void unsubscribe(QByteArray const & topic) = 0;

    virtual void publish(QByteArray const & topic, QByteArray const & msg) = 0;

signals:
    void onMessage(QByteArray const & topic, QVariant const & msg);
};

#endif // QMESSAGEPUSH_H
