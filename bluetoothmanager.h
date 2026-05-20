#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QString>

class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(int currentBPM READ currentBPM NOTIFY bpmChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    bool    isConnected() const { return m_isConnected; }
    QString deviceName()  const { return m_deviceName; }
    int     currentBPM()   const { return m_currentBPM; }

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void setMusicMode(const QString &mode);
    Q_INVOKABLE void sendCommand(const QString &command);

signals:
    void connectedChanged();
    void deviceNameChanged();
    void bpmChanged();
    void deviceDiscovered(const QString &name, const QString &address);
    void discoveryFinished();
    void noteReceived(int pitch, int velocity);
    void errorOccurred(const QString &message);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QBluetoothSocket::SocketError error);
    void onReadyRead();
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onDiscoveryFinished();

private:
    QBluetoothSocket               *m_socket;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;

    bool    m_isConnected;
    QString m_deviceName;
    int     m_currentBPM;
    QString m_dataBuffer;

    void stopAndDeleteAgent();
    void parseData(const QString &line);
};

#endif