#ifndef QEVENTBUSQML_H
#define QEVENTBUSQML_H

#include <QObject>
#include <QVariant>

class QEventBusQml : public QObject
{
    Q_OBJECT

public:
    QEventBusQml(QObject* parent = nullptr);


public slots:
    static void publish(QString what, QVariantMap value);
};

#endif // QEVENTBUSQML_H
