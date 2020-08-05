#ifndef AMP_H
#define AMP_H

#include <QObject>
#include <QString>

#include <thread>

class Amp : public QObject
{
    Q_OBJECT

public:
    explicit Amp (QObject *parent = nullptr);

signals:
protected:
};

#endif // AMP_H

