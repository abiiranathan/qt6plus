#include "BluetoothDevice.hpp"
#include <QEventLoop>
#include <QTimer>

BluetoothDevice::BluetoothDevice(QObject* parent) : QObject(parent) {
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this,
            &BluetoothDevice::deviceDiscoveredHandler);
    connect(m_socket, &QBluetoothSocket::connected, this, &BluetoothDevice::socketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &BluetoothDevice::socketDisconnected);
    connect(m_socket, &QBluetoothSocket::readyRead, this, &BluetoothDevice::socketReadyRead);
}

BluetoothDevice::~BluetoothDevice() {
    disconnectFromDevice();
}

void BluetoothDevice::discoverDevices() {
    m_discoveryAgent->start();
}

bool BluetoothDevice::connectToDevice(const QBluetoothDeviceInfo& deviceInfo,
                                      const QString& serviceUuid) {
    if (m_socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        disconnectFromDevice();
    }

    // Connect to the Bluetooth device and service
    m_socket->connectToService(deviceInfo.address(), QBluetoothUuid(serviceUuid));

    // Wait for the connected signal with a timeout
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    bool connected = false;

    QObject::connect(m_socket, &QBluetoothSocket::connected, this, [&connected, &loop]() {
        connected = true;
        loop.quit();
    });

    QObject::connect(&timer, &QTimer::timeout, this, [&loop]() { loop.quit(); });

    timer.start(10000);  // Timeout after 10 seconds
    loop.exec();

    return connected;
}

void BluetoothDevice::disconnectFromDevice() {
    m_socket->close();
}

bool BluetoothDevice::writeData(const QByteArray& data) {
    return m_socket->write(data) == data.size();
}

QByteArray BluetoothDevice::readData() {
    return m_socket->readAll();
}

void BluetoothDevice::deviceDiscoveredHandler(const QBluetoothDeviceInfo& deviceInfo) {
    emit deviceDiscovered(deviceInfo);
}

void BluetoothDevice::socketConnected() {
    emit connected();
}

void BluetoothDevice::socketDisconnected() {
    emit disconnected();
}

void BluetoothDevice::socketReadyRead() {
    emit dataReceived(readData());
}
