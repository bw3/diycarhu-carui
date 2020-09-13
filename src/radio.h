#ifndef RADIO_H
#define RADIO_H

#include <QObject>
#include <QString>

#include <thread>
#include <mutex>
#include <condition_variable>

class Radio : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString freq READ freq NOTIFY statusUpdated)
    Q_PROPERTY(int rssi READ rssi NOTIFY statusUpdated)
    Q_PROPERTY(int snr READ snr NOTIFY statusUpdated)
    Q_PROPERTY(QString radioText READ radioText NOTIFY radioTextUpdated)

    explicit Radio (QObject *parent = nullptr);
    Q_INVOKABLE void seekPrev();
    Q_INVOKABLE void seekNext();
    QString freq() { return m_freq; }
    int rssi() { return m_rssi; }
    int snr() { return m_snr; }
    QString radioText();

signals:
    void statusUpdated();
    void radioTextUpdated();

private:
    void run();
    void wait_for_bit(int bit);
    bool decode_fm_tune_status();
    int decode_rds();
    bool do_cmd();
    bool radio_text_set_char(bool,int,uint8_t);

    std::thread m_thread;
    std::condition_variable m_cmd_condition;
    std::mutex m_cmd_mutex;
    enum cmd { NONE, SEEK_NEXT, SEEK_PREV};
    cmd m_cmd;
    int i2c_fd;

    QString m_freq;
    int m_rssi;
    int m_snr;
    int m_radio_text_ab;
    char m_radio_text[65];
    char m_radio_text_inactive[65];
};

#endif // RADIO_H

