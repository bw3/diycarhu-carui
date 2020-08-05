#ifndef RADIO_H
#define RADIO_H

#include <QObject>
#include <QString>

#include <thread>

class Radio : public QObject
{
    Q_OBJECT

public:
    explicit Radio (QObject *parent = nullptr);

signals:

private:
    void wait_for_bit(int bit);
    bool decode_fm_tune_status();

    std::thread m_thread;
    int i2c_fd;
};

#endif // RADIO_H

