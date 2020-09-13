#ifndef ARDUNIO_H
#define ARDUNIO_H

#include <QObject>
#include <QString>

#include <thread>
#include <mutex>

class Arduino : public QObject
{
    Q_OBJECT
public:
    explicit Arduino (QObject *parent = nullptr);
    void run();
    Q_INVOKABLE void sendCmd(QString);

signals:
    void msg(QString event);
private:
    int uart_fd;
    std::thread m_thread;
    std::mutex writeMutex;
};

#endif // ARDUNIO_H

