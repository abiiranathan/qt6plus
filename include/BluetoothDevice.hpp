#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QObject>

#include "qt6plus_export.hpp"

class QT6PLUS_EXPORT BluetoothDevice : public QObject {
    Q_OBJECT

    QBluetoothDeviceDiscoveryAgent* m_discoveryAgent;
    QBluetoothSocket* m_socket;

   public:
    explicit BluetoothDevice(QObject* parent = nullptr);
    ~BluetoothDevice();

    void discoverDevices();
    bool connectToDevice(const QBluetoothDeviceInfo& deviceInfo, const QString& serviceUuid);
    void disconnectFromDevice();

    bool writeData(const QByteArray& data);
    QByteArray readData();

   signals:
    void deviceDiscovered(const QBluetoothDeviceInfo& deviceInfo);
    void connected();
    void disconnected();
    void dataReceived(const QByteArray& data);

   private slots:
    void deviceDiscoveredHandler(const QBluetoothDeviceInfo& deviceInfo);
    void socketConnected();
    void socketDisconnected();
    void socketReadyRead();
};

#endif  // BLUETOOTHDEVICE_H
