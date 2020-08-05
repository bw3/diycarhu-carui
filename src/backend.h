#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>

class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> menuItems READ menuItems NOTIFY menuItemsChanged)
    Q_PROPERTY(QList<QObject*> videoOptionsList READ videoOptionsList NOTIFY videoOptionsListChanged)

public:
    explicit BackEnd(QObject *parent = nullptr);

    QList<QObject*> menuItems();
    QList<QObject*> videoOptionsList();

    Q_INVOKABLE QString activateMenuItem(const QString &name);
    Q_INVOKABLE void previousMenu();
    Q_INVOKABLE void playVideo();
signals:
    void filePathChanged();
    void menuItemsChanged();
    void videoOptionsListChanged();
private:
    void adjustInputs(int num);

    std::string m_filePath;
    QList<QObject*> m_menuItems;
    VideoOptions m_videoOptions;
};

#endif // BACKEND_H
