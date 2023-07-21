/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -a bluetooth_agent -c BluetoothAgentAdapter xml
 *
 * qdbusxml2cpp is Copyright (C) 2020 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef BLUETOOTH_AGENT_H
#define BLUETOOTH_AGENT_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.bluez.Agent1
 */
class BluetoothAgentAdapter: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.bluez.Agent1\">\n"
"    <method name=\"Release\"/>\n"
"    <method name=\"RequestPinCode\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"DisplayPinCode\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"RequestPasskey\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"out\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"DisplayPasskey\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"q\"/>\n"
"    </method>\n"
"    <method name=\"RequestConfirmation\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"RequestAuthorization\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"    </method>\n"
"    <method name=\"AuthorizeService\">\n"
"      <arg direction=\"in\" type=\"o\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"Cancel\"/>\n"
"  </interface>\n"
        "")
public:
    BluetoothAgentAdapter(QObject *parent);
    virtual ~BluetoothAgentAdapter();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void AuthorizeService(const QDBusObjectPath &in0, const QString &in1);
    void Cancel();
    void DisplayPasskey(const QDBusObjectPath &in0, uint in1, ushort in2);
    void DisplayPinCode(const QDBusObjectPath &in0, const QString &in1);
    void Release();
    void RequestAuthorization(const QDBusObjectPath &in0);
    void RequestConfirmation(const QDBusObjectPath &in0, uint in1);
    uint RequestPasskey(const QDBusObjectPath &in0);
    QString RequestPinCode(const QDBusObjectPath &in0);
Q_SIGNALS: // SIGNALS
};

#endif