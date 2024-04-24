#pragma once

#include <QObject>
#include <QSerialPort>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QAbstractSocket>
#include <QTimer>

class CommunicationModule : public QObject
{
    Q_OBJECT
public:
    enum ConnectionStatus {
        NotConnected,
        NoSerialPorts,
        TerminalIsNotReady,
        Connecting,
        Connected,
        ConnectionFailed,
        Closed
    };
    Q_ENUM(ConnectionStatus)

    enum class ConnectionMode {
        Serial,
        TCP,
        UDP
    };


    explicit CommunicationModule(QObject *parent = nullptr);
    void setSerialParam(QString &portName,
                        qint32 baudRate,
                        QSerialPort::DataBits dataBits,
                        QSerialPort::Parity parity,
                        QSerialPort::StopBits stopBits,
                        QSerialPort::FlowControl flowControl);
    void setSerialParam(QString portName,
                        qint32 baudRate,
                        qint32 dataBits,
                        QString parity,
                        QString stopBits,
                        QString flowControl);
    void setConParams(int port, QString ip);

    CommunicationModule::ConnectionMode connectionMode();
    void setConnectionMode(CommunicationModule::ConnectionMode mode);
    quint64 writeData(const QByteArray &data, bool waitResponse = false);
    QByteArray writeCmd(const QByteArray &data, bool waitResponse);
    QString statusString();
    CommunicationModule::ConnectionStatus status();
    void setTimeout(int value);

private:
    static CommunicationModule *m_instance;
    QByteArray rcvdData;  // temp buffer for rcvd data
    QUdpSocket m_udpSocket;
    QTcpSocket m_tcpSocket;
    QSerialPort m_serial;
    ConnectionMode m_comMode;
    int m_port;
    QString m_ip;
    bool m_connected;
    QString m_statusString;
    CommunicationModule::ConnectionStatus m_connectionStatus;
    QTimer m_timeoutTimer;
    QTimer m_sendingTimer;
    const int TX_PERIOD = 100;
    int CONNECTION_TIMEOUT = 30000;

signals:
    void statusChanged(CommunicationModule::ConnectionStatus);
    void error(QAbstractSocket::SocketError);
    void gotData(QByteArray);

public slots:
    void connect();
    void closeConnection();

private slots:
    void readData();
    void connectionStatusChanged(CommunicationModule::ConnectionStatus conStatus);
    void getError();
    void stateChanged(QAbstractSocket::SocketState socketState);
    void comTimeout();

};
