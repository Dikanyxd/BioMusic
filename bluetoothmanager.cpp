#include "bluetoothmanager.h"
#include <QDebug>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent),
    m_socket(nullptr),
    m_discoveryAgent(nullptr),
    m_isConnected(false),
    m_currentBPM(0)
{
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);

    connect(m_socket, &QBluetoothSocket::connected,
            this, &BluetoothManager::onSocketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected,
            this, &BluetoothManager::onSocketDisconnected);
    connect(m_socket, &QBluetoothSocket::errorOccurred,
            this, &BluetoothManager::onSocketError);
    connect(m_socket, &QBluetoothSocket::readyRead,
            this, &BluetoothManager::onReadyRead);
}

BluetoothManager::~BluetoothManager()
{
    if (m_socket && m_socket->isOpen())
        m_socket->close();
}

void BluetoothManager::stopAndDeleteAgent()
{
    if (m_discoveryAgent) {
        m_discoveryAgent->stop();
        delete m_discoveryAgent;
        m_discoveryAgent = nullptr;
    }
}

void BluetoothManager::startDiscovery()
{
    qDebug() << "Starting Bluetooth discovery...";

    stopAndDeleteAgent();

    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::onDeviceDiscovered);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::onDiscoveryFinished);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, [this](QBluetoothDeviceDiscoveryAgent::Error error) {
                qDebug() << "Discovery error:" << error;
                emit errorOccurred(QStringLiteral("Ошибка поиска Bluetooth: %1").arg(error));
                emit discoveryFinished();
            });

    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QString name = device.name().trimmed();
    QString address = device.address().toString();

    if (name.isEmpty())
        name = QStringLiteral("Unknown device");

    qDebug() << "Found device:" << name << address;

    emit deviceDiscovered(name, address);
}

void BluetoothManager::onDiscoveryFinished()
{
    qDebug() << "Discovery finished";
    emit discoveryFinished();
}

void BluetoothManager::connectToDevice(const QString &address)
{
    qDebug() << "Connecting to:" << address;

    if (m_socket->state() != QBluetoothSocket::SocketState::UnconnectedState)
        m_socket->disconnectFromService();

    stopAndDeleteAgent();

    QBluetoothAddress btAddress(address);

    m_socket->connectToService(
        btAddress,
        QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort)
        );
}

void BluetoothManager::disconnectDevice()
{
    if (m_socket->state() != QBluetoothSocket::SocketState::UnconnectedState)
        m_socket->disconnectFromService();
}

void BluetoothManager::onSocketConnected()
{
    m_isConnected = true;
    m_deviceName = m_socket->peerName();

    if (m_deviceName.isEmpty())
        m_deviceName = QStringLiteral("ESP32");

    qDebug() << "Connected to:" << m_deviceName;

    emit connectedChanged();
    emit deviceNameChanged();
}

void BluetoothManager::onSocketDisconnected()
{
    m_isConnected = false;
    m_deviceName.clear();
    m_currentBPM = 0;
    m_dataBuffer.clear();

    qDebug() << "Disconnected";

    emit connectedChanged();
    emit deviceNameChanged();
    emit bpmChanged();
}

void BluetoothManager::onSocketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "Socket error:" << error << m_socket->errorString();

    m_isConnected = false;

    QString msg;
    switch (error) {
    case QBluetoothSocket::SocketError::HostNotFoundError:
        msg = "ESP32 не найден. Убедитесь, что он включён.";
        break;
    case QBluetoothSocket::SocketError::ServiceNotFoundError:
        msg = "Сервис SerialPort не найден. Перезагрузите ESP32.";
        break;
    case QBluetoothSocket::SocketError::NetworkError:
        msg = "Ошибка соединения. Попробуйте ещё раз.";
        break;
    default:
        msg = "Ошибка подключения: " + m_socket->errorString();
        break;
    }

    emit connectedChanged();
    emit errorOccurred(msg);
}

void BluetoothManager::onReadyRead()
{
    m_dataBuffer += QString::fromUtf8(m_socket->readAll());

    while (m_dataBuffer.contains('\n')) {
        int idx = m_dataBuffer.indexOf('\n');
        QString line = m_dataBuffer.left(idx).trimmed();
        m_dataBuffer = m_dataBuffer.mid(idx + 1);

        if (!line.isEmpty())
            parseData(line);
    }

    if (m_dataBuffer.size() > 512)
        m_dataBuffer.clear();
}

void BluetoothManager::parseData(const QString &line)
{
    qDebug() << "<<" << line;

    if (line.startsWith("BPM:")) {
        bool ok = false;
        int bpm = line.mid(4).toInt(&ok);
        if (ok) {
            m_currentBPM = bpm;
            emit bpmChanged();
        }
    }
    else if (line.startsWith("NOTE:")) {
        QStringList parts = line.mid(5).split(',');
        if (parts.size() == 2) {
            bool ok1 = false, ok2 = false;
            int pitch = parts[0].toInt(&ok1);
            int velocity = parts[1].toInt(&ok2);

            if (ok1 && ok2)
                emit noteReceived(pitch, velocity);
        }
    }
}

void BluetoothManager::sendCommand(const QString &command)
{
    if (m_isConnected && m_socket->isWritable()) {
        m_socket->write((command + "\n").toUtf8());
        qDebug() << ">>" << command;
    }
}

void BluetoothManager::setMusicMode(const QString &mode)
{
    if (mode == "work") {
        sendCommand("MODE_WORK");
    } else if (mode == "road") {
        sendCommand("MODE_ROAD");
    } else if (mode == "sport") {
        sendCommand("MODE_SPORT");
    }
}