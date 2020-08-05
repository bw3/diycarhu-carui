#ifndef ARDUNIO_H
#define ARDUNIO_H

#include <QObject>
#include <QString>

#include <thread>


class Arduino : public QObject
{
public:
    explicit Arduino (QObject *parent = nullptr);


signals:
private:
};

#endif // ARDUNIO_H

