#ifndef AMP_H
#define AMP_H

#include <QObject>
#include <QString>
#include <QList>

#include <thread>
#include <mutex>
#include <condition_variable>

class Amp : public QObject
{
    Q_OBJECT

public:
    explicit Amp (int i2c_adptr, uint8_t i2c_addr);
    explicit Amp (int i2c_adptr, std::vector<uint8_t> i2c_addr);
    Q_INVOKABLE void play();
    Q_INVOKABLE void mute();
    Q_INVOKABLE void hi_z();
    Q_INVOKABLE void line_outputs(QList<int> list);

signals:
protected:

private:
    void run(int i2c_adptr, std::vector<uint8_t> i2c_addr);
    void do_cmd();

    std::vector<int> i2c_fd;
    std::vector<uint8_t> i2c_addr;
    std::vector<uint8_t> line_state;
    std::thread m_thread;
    std::condition_variable m_cmd_condition;
    std::mutex m_cmd_mutex;
    enum cmd { NONE, MUTE, HI_Z, PLAY };
    cmd m_cmd;
    bool dc_diag;
};

#endif // AMP_H

