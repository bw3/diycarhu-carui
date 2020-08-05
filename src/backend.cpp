#include "backend.h"
#include "menu_option.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <QProcess>

const std::string ROOT_PATH = "/mnt/big/symlinks";
const std::string GAMES_MENU = "games";

void BackEnd::adjustInputs(int num)
{
    QProcess *pyProcess = new QProcess(this);
    QStringList arguments;
    arguments << "/home/user/bin/adjust_inputs.py";
    arguments << std::to_string(num).c_str();
    pyProcess->start("/usr/bin/python", arguments);
    pyProcess->waitForFinished(-1);
    delete pyProcess;

}

BackEnd::BackEnd(QObject *parent) :
    QObject(parent)
{
    m_filePath = "/";
    adjustInputs(0);
}

std::string fullFilePath(std::string path) {
    return ROOT_PATH + "/" + path;
}

bool includeInMenu(struct dirent* entry) {
    std::string filename(entry->d_name);
    if(filename[0] == '.') {
        return false;
    }
    if(entry->d_type == DT_DIR) {
        return true;
    }
    if(filename.rfind(".mkv") == filename.length() - 4) {
        return true;
    }
    return false;
}

std::string getSingleFilename(std::string dir_path) {
    DIR* dir_ptr = opendir( dir_path.c_str() );
    if( dir_ptr == NULL ) {
        std::cout << "NULL\n";
        return "";
    }
    std::string filename;
    int numEntries = 0;
    struct dirent* entry = readdir(dir_ptr);
    while(entry != NULL && numEntries < 2) {
        if(!includeInMenu(entry)) {
            entry = readdir(dir_ptr);
            continue;
        }
        if(entry->d_type == DT_DIR) {
            numEntries = 0;
            break;
        }
        filename = entry->d_name;
        numEntries++;
        entry = readdir(dir_ptr);
    }
    closedir(dir_ptr);
    std::cout << numEntries << "\n";
    if(numEntries == 1) {
        return "/" + filename;
    } else {
        return "";
    }
}

Q_INVOKABLE QString BackEnd::activateMenuItem(const QString &name)
{
    std::string tmp_filePath = m_filePath + "/" + name.toUtf8().toStdString();
    struct stat sb;
    if( name == GAMES_MENU.c_str() ) {
        m_filePath = tmp_filePath;
        emit menuItemsChanged();
        adjustInputs(2);
        return "menu";
    }
    if( stat(fullFilePath(tmp_filePath).c_str(), &sb) == 0 ) {
        if( S_ISREG( sb.st_mode ) ) {
            m_filePath = tmp_filePath;
            return "video";
        } 
        if( S_ISDIR( sb.st_mode ) ) {
            std::string single_filename = getSingleFilename(fullFilePath(tmp_filePath));
            m_filePath = tmp_filePath + single_filename;
            if(single_filename == "") {
                emit menuItemsChanged();
                return "menu";
            } else {
                return "video";
            }
        }
    }
    return "menu";
}

Q_INVOKABLE void BackEnd::previousMenu()
{
    if( m_filePath.substr(1) == GAMES_MENU ) {
        adjustInputs(1);
    }
    size_t slash_pos = m_filePath.rfind('/');
    if( slash_pos != std::string::npos ) {
        m_filePath.erase(slash_pos);
        emit menuItemsChanged();
    }
}

Q_INVOKABLE void BackEnd::playVideo()
{
    m_videoOptions.doAdjustment();
    adjustInputs(1);
    QProcess *mpvProcess = new QProcess(this);
    QStringList arguments;
    arguments << m_videoOptions.getMpvArguments().c_str();
    arguments << " ";
    arguments << fullFilePath(m_filePath).c_str();
    mpvProcess->start("/usr/bin/mpv", arguments);
    mpvProcess->waitForFinished(-1);
    delete mpvProcess;
}

bool comparePointer(QObject* a, QObject* b) { return (*static_cast<MenuItem*>(a)) < (*static_cast<MenuItem*>(b)); }


QList<QObject*> BackEnd::menuItems()
{
    QList<QObject*> dataList;
    std::string dir_path = ROOT_PATH + m_filePath.c_str();
    DIR* dir_ptr = opendir( dir_path.c_str() );
    if( dir_ptr == NULL ) {
        dataList.append(new MenuItem( "" ) );
        return dataList;
    }
    struct dirent* entry = readdir(dir_ptr);
    bool noDir = true;
    while( entry != NULL ) {
        if( includeInMenu(entry) ) {
            dataList.append(new MenuItem( entry->d_name ) );
            if(entry->d_type == DT_DIR) {
                noDir = false;
            }
        }
        entry = readdir(dir_ptr);
    }
    closedir(dir_ptr);
    if(dataList.size() == 1 && noDir) {
        previousMenu();
        return menuItems();
    }
    std::sort(dataList.begin(), dataList.end(), comparePointer);
    if(m_filePath == "/") {
        dataList.append(new MenuItem(GAMES_MENU.c_str()));
    }
    return (QList<QObject*>)dataList;
}

QList<QObject*> BackEnd::videoOptionsList()
{
    QList<QObject*> list;
    list.append(new MenuItem("Play"));
    for(size_t i=0; i< m_videoOptions.getListOptions().size();i++) {
        list.append(m_videoOptions.getListOptions()[i]);
    }
    return (QList<QObject*>)list;
}


