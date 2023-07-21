#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDBusInterface>

#include <thread>
#include <mutex>
#include <condition_variable>

class Bluetooth : public QObject
{
    Q_OBJECT

public:
//    Q_PROPERTY(QString freq READ freq NOTIFY statusUpdated)
//    Q_PROPERTY(int rssi READ rssi NOTIFY statusUpdated)
//    Q_PROPERTY(int snr READ snr NOTIFY statusUpdated)
//    Q_PROPERTY(QString radioText READ radioText NOTIFY radioTextUpdated)

    explicit Bluetooth (QObject *parent = nullptr);
    Q_INVOKABLE void allowPairing();
    Q_INVOKABLE void disallowPairing();
    Q_INVOKABLE QVariantMap getPairedDevices();
    Q_INVOKABLE void removeDevice(QString device);

signals:

private:
    void init();
    void run();
    bool do_cmd();

    std::thread m_thread;
    std::condition_variable m_cmd_condition;
    std::mutex m_cmd_mutex;
    enum cmd { NONE, ALLOW_PAIRING, DISALLOW_PAIRING };
    cmd m_cmd;

    QObject m_pairing_agent;
    QDBusInterface* m_bluetooth_adapter;
    QDBusInterface* m_bluetooth_agent_manager;

};

#endif // BLUETOOTH_H

