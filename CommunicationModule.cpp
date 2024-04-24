#include "CommunicationModule.h"
#include <QSerialPortInfo>
#include <QDataStream>
#include <QMetaEnum>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>


CommunicationModule::CommunicationModule(QObject *parent) : QObject(parent)
{
    //QObject::connect(&m_timeoutTimer, SIGNAL(timeout()), this, SLOT(comTimeout()));

    m_comMode = ConnectionMode::Serial;
    m_connected = false;
    m_connectionStatus = CommunicationModule::ConnectionStatus::NotConnected;

    m_tcpSocket.setParent(this);
    m_udpSocket.setParent(this);
    m_serial.setParent(this);
    m_timeoutTimer.setParent(this);
    m_sendingTimer.setParent(this);

    qRegisterMetaType<CommunicationModule::ConnectionStatus>("CommunicationModule::ConnectionStatus");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");

    QObject::connect(this, SIGNAL(statusChanged(CommunicationModule::ConnectionStatus)),
                     this, SLOT(connectionStatusChanged(CommunicationModule::ConnectionStatus)));

    //---Serial port---
    //QObject::connect(&m_serial, &QSerialPort::readyRead, this, &CommunicationModule::readData);
    QObject::connect(&m_serial, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
          this, &CommunicationModule::getError);
    //---Serial port---

    //---TCP socket---
    QObject::connect(&m_tcpSocket, &QTcpSocket::connected, [=]() {
        m_connected = true;
        emit statusChanged(CommunicationModule::ConnectionStatus::Connected);
    });
    QObject::connect(&m_tcpSocket, &QTcpSocket::readyRead,this, &CommunicationModule::readData);

    #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        QObject::connect(&m_tcpSocket, &QAbstractSocket::errorOccurred,
                         this, &CommunicationModule::error);
        QObject::connect(&m_tcpSocket, &QAbstractSocket::errorOccurred,
                         this, &CommunicationModule::getError);
    #else
        typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
        QObject::connect(&m_tcpSocket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
                         this, &CommunicationModule::error);
        QObject::connect(&m_tcpSocket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
                         this, &CommunicationModule::getError);
    #endif


    QObject::connect(&m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    //---TCP socket---

    //---UDP socket---
    QObject::connect(&m_udpSocket, &QUdpSocket::connected, [=]() {
        m_connected = true;
        emit statusChanged(CommunicationModule::ConnectionStatus::Connected);
    });
    QObject::connect(&m_udpSocket, &QUdpSocket::readyRead,this, &CommunicationModule::readData);


    #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        QObject::connect(&m_udpSocket, &QAbstractSocket::errorOccurred,
                         this, &CommunicationModule::error);
        QObject::connect(&m_udpSocket, &QAbstractSocket::errorOccurred,
                         this, &CommunicationModule::getError);
    #else
        typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
        QObject::connect(&m_udpSocket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
                         this, &CommunicationModule::error);
        QObject::connect(&m_udpSocket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
                         this, &CommunicationModule::getError);
    #endif

    QObject::connect(&m_udpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    //---UDP socket---
}

CommunicationModule::ConnectionMode CommunicationModule::connectionMode()
{
    return m_comMode;
}

void CommunicationModule::setConnectionMode(CommunicationModule::ConnectionMode mode)
{
    m_comMode = mode;
}

void CommunicationModule::readData()
{
    QByteArray data;

    if (m_comMode == ConnectionMode::Serial)
    {
        data = m_serial.readAll();
    }
    else if (m_comMode == ConnectionMode::UDP)
    {
        data = m_udpSocket.readAll();
    }
    else if (m_comMode == ConnectionMode::TCP)
    {
        data = m_tcpSocket.readAll();
    }

    #ifdef QT_DEBUG
        qDebug() << "=============GETTING DATA================================";
        qDebug() << data;
        //qDebug() << data.toHex(' ');
    #endif

    emit gotData(data);
}

quint64 CommunicationModule::writeData(const QByteArray &data, bool waitResponse)
{
    quint64 bytesWritten = 0;

    if (m_comMode == ConnectionMode::Serial)
    {
        if (m_serial.isWritable())
        {
            bytesWritten = m_serial.write(data);
            //qDebug() << this->objectName() << "writeData";  //<< data.toHex(' ');

            /*
             * SYSTEM:REMOTE
             * CONF:VOLT:DC 1
             * MEAS:volt:dc? 1
             * SYSTEM:LOCAL"
            */

            // The code at below, just for testing.
            // Usually the writeData is used for sending command without response.

            if (waitResponse)
            {
                m_serial.waitForReadyRead(5000);
                QByteArray resp = m_serial.readAll();

                QTime dieTime = QTime::currentTime().addMSecs( 100 );
                while( QTime::currentTime() < dieTime )
                {
                    QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
                }
                m_serial.waitForReadyRead(5000);
                resp = resp.append(m_serial.readAll());

                bool ok;
                double result = resp.toDouble(&ok);
                QString res;
                if (ok)
                {
                    QString str = QString("%1").arg(result);
                    res = QString::number(result, 'f', 5);
                }

                emit gotData(resp);
            }
        }
        else
        {
            connect();
        }
    }
    else if (m_comMode == ConnectionMode::UDP)
    {
        if (m_udpSocket.state() == QAbstractSocket::ConnectedState && m_udpSocket.isWritable())
            bytesWritten = m_udpSocket.write(data);
        else
            connect();
    }
    else if (m_comMode == ConnectionMode::TCP)
    {
        if (m_tcpSocket.state() == QAbstractSocket::SocketState::ConnectedState && m_tcpSocket.isWritable())
            bytesWritten = m_tcpSocket.write(data);
        else
            connect();
    }

    return bytesWritten;
}

QByteArray CommunicationModule::writeCmd(const QByteArray &data, bool waitResponse)
{
    if (m_comMode == ConnectionMode::Serial)
    {
        //if (m_serial.isDataTerminalReady())
        {
            if (m_serial.isWritable())
            {
                m_serial.write(data);
                //qDebug() << this->objectName() << "writeCmd";  //<< data.toHex(' ');

                if (waitResponse)
                {
                    m_serial.waitForReadyRead(5000);
                    QByteArray resp = m_serial.readAll();

                    QTime dieTime = QTime::currentTime().addMSecs( 500 );
                    while( QTime::currentTime() < dieTime )
                    {
                        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
                    }
                    m_serial.waitForReadyRead(5000);
                    resp = resp.append(m_serial.readAll());
                    resp = resp.replace("\r\n","");

                    bool ok;
                    double result = resp.toDouble(&ok);
                    QString res;
                    if (ok)
                    {
                        QString str = QString("%1").arg(result);
                        res = QString::number(result, 'f', 5);
                    }
                    else
                    {
                        res = resp;
                    }

                    return res.toUtf8();
                }
                else
                {
                    QByteArray resp = m_serial.readAll();

                    bool ok;
                    double result = resp.toDouble(&ok);
                    QString res;
                    if (ok)
                    {
                        QString str = QString("%1").arg(result);
                        res = QString::number(result, 'f', 5);
                    }
                    else
                    {
                        res = resp;
                    }

                    return res.toUtf8();
                }
            }
            else
            {
                qDebug() << "Not writable";
                return QString("0").toUtf8();
            }
        }
    }
    else
    {
         Q_ASSERT_X(false, "CommunicationModule", "For this communication type, action is not defined");
    }

    return QByteArray();
}

void CommunicationModule::connect()
{
    if (m_comMode == ConnectionMode::Serial)
    {
        QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
        if (list.size() < 1)
        {
            qDebug() << "Not available serial ports";
            m_connected = false;
            m_connectionStatus = CommunicationModule::ConnectionStatus::NoSerialPorts;
            emit statusChanged(m_connectionStatus);
            return;
        }


        if (m_serial.open(QIODevice::ReadWrite))
        {
            qDebug() << "Connected";
            m_connected = true;
            m_connectionStatus = CommunicationModule::ConnectionStatus::Connected;
            emit statusChanged(m_connectionStatus);
        }
        else
        {
            qDebug() << "Could not open serial port";
            m_connected = false;
            m_connectionStatus = CommunicationModule::ConnectionStatus::ConnectionFailed;
            emit statusChanged(m_connectionStatus);
            closeConnection();
        }
    }
    else
    {
        if (m_udpSocket.isOpen())
            m_udpSocket.close();

        if (m_tcpSocket.isOpen())
            m_tcpSocket.close();

        m_connectionStatus = CommunicationModule::ConnectionStatus::Connecting;
        emit statusChanged(m_connectionStatus);

        if (m_comMode == ConnectionMode::UDP)
        {
            m_udpSocket.bind(QHostAddress::Any, m_port);

            if (m_udpSocket.state() != QAbstractSocket::SocketState::ConnectedState)
                m_udpSocket.connectToHost(m_ip, m_port);
            else
                qDebug() << m_udpSocket.state();
        }

        if (m_comMode == ConnectionMode::TCP)
        {
            if (m_tcpSocket.state() != QAbstractSocket::SocketState::ConnectedState)
                m_tcpSocket.connectToHost(m_ip, m_port);
            else
                qDebug() << m_tcpSocket.state();
        }
    }
}

void CommunicationModule::closeConnection()
{
    if (m_comMode == ConnectionMode::Serial && m_serial.isOpen())
        m_serial.close();

    if (m_comMode == ConnectionMode::UDP && m_udpSocket.isOpen())
        m_udpSocket.close();

    if (m_comMode == ConnectionMode::TCP && m_tcpSocket.isOpen())
        m_tcpSocket.close();

    m_connected = false;
    m_connectionStatus = CommunicationModule::ConnectionStatus::Closed;

    emit statusChanged(m_connectionStatus);
}

void CommunicationModule::getError()
{
    QString errorStr;
    if (m_comMode == ConnectionMode::Serial)
        errorStr = m_serial.errorString() + " " + m_serial.error();

    if (m_comMode == ConnectionMode::UDP)
    {
        errorStr = m_udpSocket.errorString() + " " + m_udpSocket.error();
        m_udpSocket.abort();
    }
    if (m_comMode == ConnectionMode::TCP)
    {
        errorStr = m_tcpSocket.errorString() + " " + m_tcpSocket.error();
        m_tcpSocket.abort();
    }
    qDebug() << errorStr << this->objectName();
}

void CommunicationModule::setSerialParam(QString &portName,
                                         qint32 baudRate,
                                         QSerialPort::DataBits dataBits,
                                         QSerialPort::Parity parity,
                                         QSerialPort::StopBits stopBits,
                                         QSerialPort::FlowControl flowControl)
{
    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(dataBits);
    m_serial.setParity(parity);
    m_serial.setStopBits(stopBits);
    m_serial.setFlowControl(flowControl);
}

void CommunicationModule::setSerialParam(QString portName,
                    qint32 baudRate,
                    qint32 dataBits,
                    QString parity,
                    QString stopBits,
                    QString flowControl)
{
    QSerialPort::Parity prty = QSerialPort::NoParity;
    if (parity == "None")
        prty = QSerialPort::NoParity;
    if (parity == "Even")
        prty = QSerialPort::EvenParity;
    if (parity == "Odd")
        prty = QSerialPort::OddParity;
    if (parity == "Mark")
        prty = QSerialPort::MarkParity;
    if (parity == "Space")
        prty = QSerialPort::SpaceParity;

    QSerialPort::StopBits stpBits = QSerialPort::OneStop;
    if (stopBits == "1")
        stpBits = QSerialPort::OneStop;
    if (stopBits == "1.5")
        stpBits = QSerialPort::OneAndHalfStop;
    if (stopBits == "2")
        stpBits = QSerialPort::TwoStop;

    QSerialPort::FlowControl flowCtrl = QSerialPort::NoFlowControl;
    if (flowControl == "None")
        flowCtrl = QSerialPort::NoFlowControl;
    if (flowControl == "RTS/CTS")
        flowCtrl = QSerialPort::HardwareControl;
    if (flowControl == "XON/XOFF")
        flowCtrl = QSerialPort::SoftwareControl;

    setSerialParam(portName,
                   baudRate,
                   static_cast<QSerialPort::DataBits>(dataBits),
                   prty,
                   stpBits,
                   flowCtrl
                );
}

void CommunicationModule::setConParams(int port, QString ip)
{
    m_port  = port;
    m_ip    = ip;
}

QString CommunicationModule::statusString()
{
    return m_statusString;
}

void CommunicationModule::stateChanged(QAbstractSocket::SocketState socketState)
{
    Q_UNUSED(socketState);
}

CommunicationModule::ConnectionStatus CommunicationModule::status()
{
    return m_connectionStatus;
}

void CommunicationModule::connectionStatusChanged(CommunicationModule::ConnectionStatus conStatus)
{
    m_connectionStatus = conStatus;

    switch(conStatus) {
    case NoSerialPorts:
        m_statusString = "There is no serial ports";
        break;
    case TerminalIsNotReady:
        m_statusString = "Terminal is not ready";
        break;
    case Connecting:
        m_statusString = "Connecting...";
        break;
    case Connected:
        if (m_comMode == ConnectionMode::Serial)
        {
            m_statusString = QString("Connected to %1 : %2, %3, %4, %5, %6")
                              .arg(m_serial.portName())
                              .arg(m_serial.baudRate())
                              .arg(m_serial.dataBits())
                              .arg(m_serial.parity())
                              .arg(m_serial.stopBits())
                              .arg(m_serial.flowControl());
        }
        else if (m_comMode == ConnectionMode::UDP)
        {
            m_statusString = QString("Connected to %1 : %2")
                                      .arg(m_ip)
                                      .arg(m_port);
        }
        else if (m_comMode == ConnectionMode::TCP)
        {
            m_statusString = QString("Connected to %1 : %2")
                                      .arg(m_ip)
                                      .arg(m_port);
        }

        m_timeoutTimer.start(CONNECTION_TIMEOUT);
        break;
    case ConnectionFailed:
        m_statusString = "Connection failed";
        break;
    case ConnectionStatus::Closed:
        m_statusString = "Connection closed";
        break;
    default: {
        QMetaEnum metaEnum = QMetaEnum::fromType<CommunicationModule::ConnectionStatus>();
        QString statusStr = metaEnum.valueToKey(conStatus);

        QString msg = "undefined connection status: " + statusStr;
        Q_ASSERT_X(false, "ERROR",  msg.toLocal8Bit().data());
    }
    }
}

void CommunicationModule::comTimeout()
{
    qDebug() <<"**** WE DONT GET RESPONSE IN timeout period - do reconnection";

    closeConnection();
    connect();
}

void CommunicationModule::setTimeout(int value)
{
    CONNECTION_TIMEOUT = value;
}
