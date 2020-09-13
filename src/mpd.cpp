#include "mpd.h"
#include <mpd/client.h>

#include <chrono>

Mpd::Mpd (QObject *parent) {
    m_cmd = NONE;
    std::thread t(&Mpd::run, this);
    m_thread.swap(t);
}

void Mpd::run() {
    m_conn = mpd_connection_new("127.0.0.1", 0, 0);
    // error handling
    if (m_conn == NULL) {
        fprintf(stderr, "Out of memory\n");
        return;
    }
    if (mpd_connection_get_error(m_conn) != MPD_ERROR_SUCCESS) {
        fprintf(stderr, "%s\n", mpd_connection_get_error_message(m_conn));
        mpd_connection_free(m_conn);
        return;
    }

    while(true) {
        {
            std::unique_lock<std::mutex> l(m_cmd_mutex);
            if(m_cmd == NONE) {
                using namespace std::chrono;
                m_cmd_condition.wait_for(l, 500ms);
            }
        }
        do_cmd();
        m_artist = "";
        m_title = "";
        struct mpd_status* status = mpd_run_status( m_conn );
        if( status != NULL ) {
            int curSongId = mpd_status_get_song_id( status );
            if( curSongId != -1 ) {
                struct mpd_song* curSong = mpd_run_get_queue_song_id( m_conn, curSongId );
                if( curSong != NULL) {
                    const char* title = mpd_song_get_tag(curSong, MPD_TAG_TITLE, 0);
                    if( title != NULL ) {
                        m_title = title;
                    }
                    const char* artist = mpd_song_get_tag(curSong, MPD_TAG_ARTIST, 0);
                    if( artist != NULL ) {
                        m_artist = artist;
                    }
                    mpd_song_free(curSong);
                }
            }
            mpd_status_free(status);
        }
        emit statusUpdated();
    }
}

void Mpd::do_cmd() {
    enum cmd c;
    {
        std::lock_guard l(m_cmd_mutex);
        c = m_cmd;
    }
    switch(c) {
        case NEXT:
            mpd_run_next(m_conn);
            break;
        case PREV:
            mpd_run_previous(m_conn);
            break;
        case RESUME:
            mpd_run_pause(m_conn,false);
            break;
        case PAUSE:
            mpd_run_pause(m_conn,true);
            break;
        case TOGGLE_PAUSE:
            mpd_run_toggle_pause(m_conn);
            break;
        case NONE:
            break;
    }
    m_cmd = NONE;
}

void Mpd::next() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = NEXT;
    m_cmd_condition.notify_one();
}

void Mpd::prev() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = PREV;
    m_cmd_condition.notify_one();
}

void Mpd::resume() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = RESUME;
    m_cmd_condition.notify_one();
}

void Mpd::pause() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = PAUSE;
    m_cmd_condition.notify_one();
}

void Mpd::toggle_pause() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = TOGGLE_PAUSE;
    m_cmd_condition.notify_one();
}

