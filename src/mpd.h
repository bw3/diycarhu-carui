#ifndef MPD_H
#define MPD_H

#include <QObject>
#include <QString>

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <mpd/connection.h>

class Mpd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY statusUpdated)
    Q_PROPERTY(QString artist READ artist NOTIFY statusUpdated)
public:
    explicit Mpd (QObject *parent = nullptr);
    void run();

    void do_cmd();
    Q_INVOKABLE void prev();
    Q_INVOKABLE void next();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void toggle_pause();
    Q_INVOKABLE void play_all_random();

    QString title() { return m_title; }
    QString artist() { return m_artist; }

signals:
    void statusUpdated();
private:
    struct mpd_connection* m_conn;
    std::thread m_thread;
    std::condition_variable m_cmd_condition;
    std::mutex m_cmd_mutex;
    enum cmd { NONE, NEXT, PREV, RESUME, PAUSE, TOGGLE_PAUSE, PLAY_ALL_RANDOM};
    std::queue<cmd> m_cmd_queue;

    QString m_artist;
    QString m_title;
};

#endif // MPD_H

