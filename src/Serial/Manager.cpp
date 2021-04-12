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

#include <Serial/Manager.h>
#include <Misc/Utilities.h>

using namespace Serial;

/**
 * Pointer to the only instance of the class.
 */
static Manager *INSTANCE = nullptr;

/**
 * Constructor function
 */
Manager::Manager()
    : m_port(nullptr)
    , m_portIndex(0)
{
    // Init serial port configuration variables
    disconnectDevice();
    setDataBits(dataBitsList().indexOf("8"));
    setStopBits(stopBitsList().indexOf("1"));
    setParity(parityList().indexOf(tr("None")));
    setBaudRateIndex(baudRateList().indexOf("9600"));
    setFlowControl(flowControlList().indexOf(tr("None")));

    // Refresh serial devices @ 1 Hz
    connect(&m_refreshTimer, &QTimer::timeout, this, &Manager::refreshSerialDevices);
    m_refreshTimer.start(1000);

    // Log class init
    qDebug() << "Class initialized";
}

/**
 * Destructor function, closes the serial port before exiting the application and saves
 * the user's baud rate list settings.
 */
Manager::~Manager()
{
    if (port())
        disconnectDevice();
}

/**
 * Returns the only instance of the class
 */
Manager *Manager::getInstance()
{
    if (INSTANCE == nullptr)
        INSTANCE = new Manager;

    return INSTANCE;
}

/**
 * Returns the pointer to the current serial port handler
 */
QSerialPort *Manager::port() const
{
    return m_port;
}

/**
 * Returns @c true if the serial device is open
 */
bool Manager::connected() const {
    if (port())
        return port()->isOpen();

    return false;
}

/**
 * Returns the name of the current serial port device
 */
QString Manager::portName() const
{
    if (port())
        return port()->portName();

    return tr("No Device");
}

/**
 * Returns @c true if the user selects the appropiate controls & options to be able
 * to connect to a serial device
 */
bool Manager::configurationOk() const
{
    return portIndex() > 0;
}

/**
 * Returns the index of the current serial device selected by the program.
 */
quint8 Manager::portIndex() const
{
    return m_portIndex;
}

/**
 * Returns the correspoding index of the parity configuration in relation
 * to the @c QStringList returned by the @c parityList() function.
 */
quint8 Manager::parityIndex() const
{
    return m_parityIndex;
}

/**
 * Returns the correspoding index of the baud rate index in relation
 * to the @c QStringList returned by the @c baudRateList() function.
 */
quint8 Manager::baudRateIndex() const
{
    return m_baudRateIndex;
}

/**
 * Returns the correspoding index of the data bits configuration in relation
 * to the @c QStringList returned by the @c dataBitsList() function.
 */
quint8 Manager::dataBitsIndex() const
{
    return m_dataBitsIndex;
}

/**
 * Returns the correspoding index of the stop bits configuration in relation
 * to the @c QStringList returned by the @c stopBitsList() function.
 */
quint8 Manager::stopBitsIndex() const
{
    return m_stopBitsIndex;
}

/**
 * Returns the correspoding index of the flow control config. in relation
 * to the @c QStringList returned by the @c flowControlList() function.
 */
quint8 Manager::flowControlIndex() const
{
    return m_flowControlIndex;
}

/**
 * Returns a list with the available serial devices/ports to use.
 * This function can be used with a combo box to build nice UIs.
 *
 * @note The first item of the list will be invalid, since it's value will
 *       be "Select Serial Device". This is inteded to make the user interface
 *       a little more friendly.
 */
QStringList Manager::portList() const
{
    return m_portList;
}

/**
 * Returns a list with the available parity configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList Manager::parityList() const
{
    QStringList list;
    list.append(tr("None"));
    list.append(tr("Even"));
    list.append(tr("Odd"));
    list.append(tr("Space"));
    list.append(tr("Mark"));
    return list;
}

/**
 * Returns a list with the available baud rate configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList Manager::baudRateList() const
{
    return QStringList { "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200" };
}

/**
 * Returns a list with the available data bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList Manager::dataBitsList() const
{
    return QStringList { "5", "6", "7", "8" };
}

/**
 * Returns a list with the available stop bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList Manager::stopBitsList() const
{
    return QStringList { "1", "1.5", "2" };
}

/**
 * Returns a list with the available flow control configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList Manager::flowControlList() const
{
    QStringList list;
    list.append(tr("None"));
    list.append("RTS/CTS");
    list.append("XON/XOFF");
    return list;
}

/**
 * Returns the current parity configuration used by the serial port
 * handler object.
 */
QSerialPort::Parity Manager::parity() const
{
    return m_parity;
}

/**
 * Returns the current baud rate configuration used by the serial port
 * handler object.
 */
qint32 Manager::baudRate() const
{
    return m_baudRate;
}

/**
 * Returns the current data bits configuration used by the serial port
 * handler object.
 */
QSerialPort::DataBits Manager::dataBits() const
{
    return m_dataBits;
}

/**
 * Returns the current stop bits configuration used by the serial port
 * handler object.
 */
QSerialPort::StopBits Manager::stopBits() const
{
    return m_stopBits;
}

/**
 * Returns the current flow control configuration used by the serial
 * port handler object.
 */
QSerialPort::FlowControl Manager::flowControl() const
{
    return m_flowControl;
}

/**
 * Tries to write the given @a data to the current device. Upon data write, the class
 * emits the @a tx() signal for UI updating.
 *
 * @returns the number of bytes written to the target device
 */
qint64 Manager::writeData(const QByteArray &data)
{
    if (connected())
    {
        qint64 bytes = port()->write(data);

        if (bytes > 0)
        {
            auto writtenData = data;
            writtenData.chop(data.length() - bytes);

            emit tx();
            emit dataSent(writtenData);
        }

        return bytes;
    }

    return -1;
}

/**
 * Tries to open the serial port with the current configuration
 */
void Manager::connectDevice()
{
    // Ignore the first item of the list (Select Port)
    auto ports = validPorts();
    auto portId = portIndex() - 1;
    if (portId >= 0 && portId < validPorts().count())
    {
        // Update port index variable & disconnect from current serial port
        disconnectDevice();
        m_portIndex = portId + 1;
        emit portIndexChanged();

        // Create new serial port handler
        m_port = new QSerialPort(ports.at(portId));

        // Configure serial port
        port()->setParity(parity());
        port()->setBaudRate(baudRate());
        port()->setDataBits(dataBits());
        port()->setStopBits(stopBits());
        port()->setFlowControl(flowControl());

        // Connect signals/slots
        connect(port(), &QIODevice::readyRead, this, &Manager::onDataReceived);
        connect(port(), SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this,
                SLOT(handleError(QSerialPort::SerialPortError)));

        // Try to open the serial port device
        if (port()->open(QIODevice::ReadWrite))
            qDebug() << "Connected to" << portName();
        else
            qWarning() << "Serial port connection error";

        // Change serial port connection status
        emit connectedChanged();
    }

    // Disconnect serial port
    else
        disconnectDevice();
}

/**
 * Disconnects from the current serial device and clears temp. data
 */
void Manager::disconnectDevice()
{
    // Check if serial port pointer is valid
    if (port() != nullptr)
    {
        // Get serial port name (used for warning messages)
        auto name = portName();

        // Disconnect signals/slots
        port()->disconnect(this, SLOT(onDataReceived()));
        port()->disconnect(this, SLOT(handleError(QSerialPort::SerialPortError)));

        // Close & delete serial port handler
        port()->close();
        port()->deleteLater();

        // Log changes
        qDebug() << "Disconnected from" << name;
    }

    // Reset pointer
    m_port = nullptr;
    emit portChanged();
    emit connectedChanged();
    emit availablePortsChanged();
}

/**
 * Connects/disconnects the application from the currently selected device. This function
 * is used as a convenience for the connect/disconnect button.
 */
void Manager::toggleConnection()
{
    if (connected())
        disconnectDevice();
    else
        connectDevice();
}

/**
 * Changes the baud @a rate of the serial port
 */
void Manager::setBaudRate(const qint32 rate)
{
    // Asserts
    Q_ASSERT(rate > 10);

    // Update baud rate
    m_baudRate = rate;

    // Update serial port config
    if (port())
        port()->setBaudRate(baudRate());

    // Update user interface
    emit baudRateChanged();

    // Log information
    qDebug() << "Baud rate set to" << rate;
}

/**
 * Changes the port index value, this value is later used by the @c openSerialPort()
 * function.
 */
void Manager::setPortIndex(const quint8 portIndex)
{
    auto portId = portIndex - 1;
    if (portId >= 0 && portId < validPorts().count())
        m_portIndex = portIndex;
    else
        m_portIndex = 0;

    emit portIndexChanged();
}

/**
 * Changes the baud rate of the serial port using the baud rate specified at the given @a
 * index
 */
void Manager::setBaudRateIndex(const int index)
{
    if (index >= 0 && index <= baudRateList().count())
    {
        m_baudRateIndex = index;
        setBaudRate(baudRateList().at(index).toInt());
        emit baudRateIndexChanged();
    }
}

/**
 * @brief Serial::setParity
 * @param parityIndex
 */
void Manager::setParity(const quint8 parityIndex)
{
    // Argument verification
    Q_ASSERT(parityIndex < parityList().count());

    // Update current index
    m_parityIndex = parityIndex;

    // Set parity based on current index
    switch (parityIndex)
    {
    case 0:
        m_parity = QSerialPort::NoParity;
        break;
    case 1:
        m_parity = QSerialPort::EvenParity;
        break;
    case 2:
        m_parity = QSerialPort::OddParity;
        break;
    case 3:
        m_parity = QSerialPort::SpaceParity;
        break;
    case 4:
        m_parity = QSerialPort::MarkParity;
        break;
    default:
        m_parity = QSerialPort::UnknownParity;
        break;
    }

    // Update serial port config.
    if (port())
        port()->setParity(parity());

    // Notify user interface
    emit parityChanged();

    // Log changes
    qDebug() << "Serial port parity set to" << parity();
}

/**
 * Changes the data bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void Manager::setDataBits(const quint8 dataBitsIndex)
{
    // Argument verification
    Q_ASSERT(dataBitsIndex < dataBitsList().count());

    // Update current index
    m_dataBitsIndex = dataBitsIndex;

    // Obtain data bits value from current index
    switch (dataBitsIndex)
    {
    case 0:
        m_dataBits = QSerialPort::Data5;
        break;
    case 1:
        m_dataBits = QSerialPort::Data6;
        break;
    case 2:
        m_dataBits = QSerialPort::Data7;
        break;
    case 3:
        m_dataBits = QSerialPort::Data8;
        break;
    default:
        m_dataBits = QSerialPort::UnknownDataBits;
        break;
    }

    // Update serial port configuration
    if (port())
        port()->setDataBits(dataBits());

    // Update user interface
    emit dataBitsChanged();

    // Log changes
    qDebug() << "Data bits set to" << dataBits();
}

/**
 * Changes the stop bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void Manager::setStopBits(const quint8 stopBitsIndex)
{
    // Argument verification
    Q_ASSERT(stopBitsIndex < stopBitsList().count());

    // Update current index
    m_stopBitsIndex = stopBitsIndex;

    // Obtain stop bits value from current index
    switch (stopBitsIndex)
    {
    case 0:
        m_stopBits = QSerialPort::OneStop;
        break;
    case 1:
        m_stopBits = QSerialPort::OneAndHalfStop;
        break;
    case 2:
        m_stopBits = QSerialPort::TwoStop;
        break;
    default:
        m_stopBits = QSerialPort::UnknownStopBits;
        break;
    }

    // Update serial port configuration
    if (port())
        port()->setStopBits(stopBits());

    // Update user interface
    emit stopBitsChanged();

    // Log changes
    qDebug() << "Stop bits set to" << stopBits();
}

/**
 * Changes the flow control option of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void Manager::setFlowControl(const quint8 flowControlIndex)
{
    // Argument verification
    Q_ASSERT(flowControlIndex < flowControlList().count());

    // Update current index
    m_flowControlIndex = flowControlIndex;

    // Obtain flow control value from current index
    switch (flowControlIndex)
    {
    case 0:
        m_flowControl = QSerialPort::NoFlowControl;
        break;
    case 1:
        m_flowControl = QSerialPort::HardwareControl;
        break;
    case 2:
        m_flowControl = QSerialPort::SoftwareControl;
        break;
    case 3:
        m_flowControl = QSerialPort::UnknownFlowControl;
        break;
    }

    // Update serial port configuration
    if (port())
        port()->setFlowControl(flowControl());

    // Update user interface
    emit flowControlChanged();

    // Log changes
    qDebug() << "Flow control set to" << flowControl();
}

/**
 * Reads incoming data from the serial device & sends a Qt signal with
 * the received data.
 */
void Manager::onDataReceived()
{
    // Verify that device is still valid
    if (!port())
        disconnectDevice();

    // Read data all incoming data from serial port
    auto data = port()->readAll();

    // Notify user interface
    emit dataReceived(data);
    emit rx();
}

/**
 * Scans for new serial ports available & generates a QStringList with current
 * serial ports.
 */
void Manager::refreshSerialDevices()
{
    // Create device list, starting with dummy header
    // (for a more friendly UI when no devices are attached)
    QStringList ports;
    ports.append(tr("Select Port"));

    // Search for available ports and add them to the lsit
    auto validPortList = validPorts();
    foreach (QSerialPortInfo info, validPortList)
    {
        if (!info.isNull())
        {
            // Get port name & description
            auto name = info.portName();
            auto description = info.description();

            // Add port description (if possible)
            if (!description.isEmpty())
                ports.append(description);

            // Only add port name
            else
                ports.append(name);
        }
    }

    // Update list only if necessary
    if (portList() != ports)
    {
        // Update list
        m_portList = ports;

        // Update current port index
        if (port())
        {
            auto name = port()->portName();
            for (int i = 0; i < validPortList.count(); ++i)
            {
                auto info = validPortList.at(i);
                if (info.portName() == name)
                {
                    m_portIndex = i + 1;
                    break;
                }
            }
        }

        // Update UI
        emit availablePortsChanged();
    }
}

/**
 * @brief Serial::handleError
 * @param error
 */
void Manager::handleError(QSerialPort::SerialPortError error)
{
    qDebug() << "Serial port error" << port()->error();

    if (error != QSerialPort::NoError)
    {
        auto errorStr = port()->errorString();
        Manager::getInstance()->disconnectDevice();
        Misc::Utilities::showMessageBox(tr("Critical serial port error"), errorStr);
    }
}

/**
 * Returns a list with all the valid serial port objects
 */
QList<QSerialPortInfo> Manager::validPorts() const
{
    // Search for available ports and add them to the lsit
    QList<QSerialPortInfo> ports;
    foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        if (!info.isNull())
        {
            // Only accept *.cu devices on macOS (remove *.tty)
            // https://stackoverflow.com/a/37688347
#ifdef Q_OS_MACOS
            if (info.portName().toLower().startsWith("tty."))
                continue;
#endif
            // Append port to list
            ports.append(info);
        }
    }

    // Return list
    return ports;
}
