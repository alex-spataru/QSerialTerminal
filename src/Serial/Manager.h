/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QtSerialPort>

namespace Serial
{
class Manager : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString portName
               READ portName
               NOTIFY portChanged)
    Q_PROPERTY(bool connected
               READ connected
               NOTIFY connectedChanged)
    Q_PROPERTY(bool configurationOk
               READ configurationOk
               NOTIFY portIndexChanged)
    Q_PROPERTY(quint8 portIndex
               READ portIndex
               WRITE setPortIndex
               NOTIFY portIndexChanged)
    Q_PROPERTY(quint8 parityIndex
               READ parityIndex
               WRITE setParity
               NOTIFY parityChanged)
    Q_PROPERTY(quint8 dataBitsIndex
               READ dataBitsIndex
               WRITE setDataBits
               NOTIFY dataBitsChanged)
    Q_PROPERTY(quint8 stopBitsIndex
               READ stopBitsIndex
               WRITE setStopBits
               NOTIFY stopBitsChanged)
    Q_PROPERTY(quint8 flowControlIndex
               READ flowControlIndex
               WRITE setFlowControl
               NOTIFY flowControlChanged)
    Q_PROPERTY(quint8 baudRateIndex
               READ baudRateIndex
               WRITE setBaudRateIndex
               NOTIFY baudRateIndexChanged)
    Q_PROPERTY(qint32 baudRate
               READ baudRate
               WRITE setBaudRate
               NOTIFY baudRateChanged)
    Q_PROPERTY(QStringList portList
               READ portList
               NOTIFY availablePortsChanged)
    Q_PROPERTY(QStringList parityList
               READ parityList
               CONSTANT)
    Q_PROPERTY(QStringList baudRateList
               READ baudRateList
               NOTIFY baudRateListChanged)
    Q_PROPERTY(QStringList dataBitsList
               READ dataBitsList
               CONSTANT)
    Q_PROPERTY(QStringList stopBitsList
               READ stopBitsList
               CONSTANT)
    Q_PROPERTY(QStringList flowControlList
               READ flowControlList
               CONSTANT)
    // clang-format on

signals:
    void tx();
    void rx();
    void closed();
    void portChanged();
    void parityChanged();
    void baudRateChanged();
    void dataBitsChanged();
    void stopBitsChanged();
    void connectedChanged();
    void portIndexChanged();
    void flowControlChanged();
    void baudRateListChanged();
    void baudRateIndexChanged();
    void availablePortsChanged();
    void dataSent(const QByteArray &data);
    void connectionError(const QString &name);
    void dataReceived(const QByteArray &data);

public:
    static Manager *getInstance();

    bool connected() const;
    QString portName() const;
    QSerialPort *port() const;
    bool configurationOk() const;

    quint8 portIndex() const;
    quint8 parityIndex() const;
    quint8 displayMode() const;
    quint8 baudRateIndex() const;
    quint8 dataBitsIndex() const;
    quint8 stopBitsIndex() const;
    quint8 flowControlIndex() const;

    QStringList portList() const;
    QStringList parityList() const;
    QStringList baudRateList() const;
    QStringList dataBitsList() const;
    QStringList stopBitsList() const;
    QStringList flowControlList() const;

    qint32 baudRate() const;
    QSerialPort::Parity parity() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::FlowControl flowControl() const;

    Q_INVOKABLE qint64 writeData(const QByteArray &data);

public slots:
    void connectDevice();
    void disconnectDevice();
    void toggleConnection();
    void setBaudRate(const qint32 rate);
    void setBaudRateIndex(const int index);
    void setParity(const quint8 parityIndex);
    void setPortIndex(const quint8 portIndex);
    void setDataBits(const quint8 dataBitsIndex);
    void setStopBits(const quint8 stopBitsIndex);
    void setFlowControl(const quint8 flowControlIndex);

private slots:
    void onDataReceived();
    void refreshSerialDevices();
    void handleError(QSerialPort::SerialPortError error);

private:
    Manager();
    ~Manager();
    QList<QSerialPortInfo> validPorts() const;

private:
    QSerialPort *m_port;

    QTimer m_refreshTimer;

    qint32 m_baudRate;
    QSettings m_settings;
    QSerialPort::Parity m_parity;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;

    quint8 m_portIndex;
    quint8 m_parityIndex;
    quint8 m_dataBitsIndex;
    quint8 m_stopBitsIndex;
    quint8 m_baudRateIndex;
    quint8 m_flowControlIndex;

    QStringList m_portList;
};
}

#endif
