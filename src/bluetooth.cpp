#include "bluetooth.h"
#include "bluetooth_agent.h"
#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <chrono>
#include <regex>

Bluetooth::Bluetooth (QObject *parent ) {
    m_cmd = NONE;
    init();
    std::thread t(&Bluetooth::run, this);
    m_thread.swap(t);
}

void Bluetooth::allowPairing() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = ALLOW_PAIRING;
    m_cmd_condition.notify_one();
}
 
void Bluetooth::disallowPairing() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = DISALLOW_PAIRING;
    m_cmd_condition.notify_one();
}

void Bluetooth::init() {
    new BluetoothAgentAdapter(&m_pairing_agent);
    if(!QDBusConnection::systemBus().registerObject("/org/diycarhu", &m_pairing_agent)) {
        printf("failed to register agent object\n");
    }
}

void Bluetooth::run() {
    m_bluetooth_agent_manager = new QDBusInterface("org.bluez","/org/bluez","org.bluez.AgentManager1",QDBusConnection::systemBus());
    QDBusMessage msg{m_bluetooth_agent_manager->call("RegisterAgent",QDBusObjectPath("/org/diycarhu"),"NoInputNoOutput")};
    if(msg.type() == QDBusMessage::ErrorMessage) {
        printf("failed to register agent with agent manager\n");
        fflush(stdout);
    }
    msg = m_bluetooth_agent_manager->call("RequestDefaultAgent",QDBusObjectPath("/org/diycarhu"));
    if(msg.type() == QDBusMessage::ErrorMessage) {
        printf("failed to RequestDefaultAgent\n");
        fflush(stdout);
    }

    m_bluetooth_adapter = new QDBusInterface("org.bluez","/org/bluez/hci0","org.bluez.Adapter1",QDBusConnection::systemBus());
    while(!m_bluetooth_adapter->isValid() || !m_bluetooth_adapter->property("Powered").isValid()) {
        printf("Could not find bluetooth adapter, retrying\n");
        fflush(stdout);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
        m_bluetooth_adapter = new QDBusInterface("org.bluez","/org/bluez/hci0","org.bluez.Adapter1",QDBusConnection::systemBus());
    }
    printf("Powering on bluetooth adapter\n");
    fflush(stdout);
    m_bluetooth_adapter->setProperty("Pairable",false);
    m_bluetooth_adapter->setProperty("Powered",true);
    while(true) {
        std::unique_lock<std::mutex> l(m_cmd_mutex);
        if(m_cmd == ALLOW_PAIRING) {
            m_bluetooth_adapter->setProperty("Discoverable",true);
            m_bluetooth_adapter->setProperty("Pairable",true);
        }
        if(m_cmd == DISALLOW_PAIRING) {
            m_bluetooth_adapter->setProperty("Discoverable",false);
            m_bluetooth_adapter->setProperty("Pairable",false);
        }
        if(m_cmd == NONE) {
            using namespace std::chrono_literals;
            m_cmd_condition.wait_for(l, 1s);
        } else {
            m_cmd = NONE;
        }
    }
}

QVariantMap Bluetooth::getPairedDevices() {
    QVariantMap map;
    QDBusInterface introspect_interface("org.bluez","/org/bluez/hci0","org.freedesktop.DBus.Introspectable",QDBusConnection::systemBus());
    QDBusMessage reply = introspect_interface.call("Introspect");
    std::string xml = reply.arguments().at(0).toString().toStdString();
    std::cout << xml << "\n\n\n";
    std::regex device_regex("<node name=\"(dev_[A-Z0-9_]*)\"/>");
    std::smatch sm;
    while(std::regex_search(xml,sm,device_regex)) {
        for(size_t i=0;i<sm.size();i++) {
            std::cout << "[" << sm[i] << "]\n";
        }
        std::string device_path = "/org/bluez/hci0/";
        device_path += sm[1];
        QDBusInterface device_interface("org.bluez",device_path.c_str(),"org.bluez.Device1",QDBusConnection::systemBus());
        std::string name = device_interface.property("Alias").toString().toStdString();
        map.insert(name.c_str(),device_path.c_str());
        xml = sm.suffix().str();
    }
    return map;
}

void Bluetooth::removeDevice(QString device_path) {
    m_bluetooth_adapter->call("RemoveDevice",QDBusObjectPath(device_path));
}
