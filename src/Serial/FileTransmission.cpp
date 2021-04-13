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
#include <Serial/FileTransmission.h>

#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>

using namespace Serial;

/*
 * Only instance of the class
 */
static FileTransmission *INSTANCE = Q_NULLPTR;

/**
 * Constructor function
 */
FileTransmission::FileTransmission()
{
    // Set stream object pointer to null
    m_stream = Q_NULLPTR;

    // Send a line to the serial device periodically
    m_timer.setInterval(10);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendLine);

    // Stop transmission if serial device is disconnected
    auto mgr = Manager::getInstance();
    connect(mgr, &Manager::closed, this, &FileTransmission::stopTransmission);

    // Refresh UI when serial device connection status changes
    connect(mgr, &Manager::connectedChanged, this, &FileTransmission::fileChanged);

    // Close file before application quits
    connect(qApp, &QApplication::aboutToQuit, this, &FileTransmission::closeFile);
}

/**
 * Destructor function
 */
FileTransmission::~FileTransmission()
{
    closeFile();
}

/**
 * Returns the only instance of the class
 */
FileTransmission *FileTransmission::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new FileTransmission;

    return INSTANCE;
}

/**
 * Returns @c true if the application is currently transmitting a file through
 * the serial port.
 */
bool FileTransmission::active() const
{
    return m_timer.isActive();
}

/**
 * Returns @c true if a file is currently selected for transmission and if the
 * serial port device is available.
 */
bool FileTransmission::fileOpen() const
{
    return m_file.isOpen() && Manager::getInstance()->connected();
}

/**
 * Returns the name & extension of the currently selected file
 */
QString FileTransmission::fileName() const
{
    if (!m_file.isOpen())
        return tr("No file selected...");

    return QFileInfo(m_file).fileName();
}

/**
 * Returns the file transmission progress in a range from 0 to 100
 */
int FileTransmission::transmissionProgress() const
{
    // No file open or invalid size -> progress set to 0%
    if (!fileOpen() || m_file.size() <= 0 || !m_stream)
        return 0;

    // Return progress as percentage
    qreal txb = m_stream->pos();
    qreal len = m_file.size();
    return qMin(1.0, (txb / len)) * 100;
}

/**
 * Returns the number of milliseconds that the application should wait before
 * sending another line to the serial port device.
 */
int FileTransmission::lineTransmissionInterval() const
{
    return m_timer.interval();
}

/**
 * Allows the user to select a file to send to the serial port.
 */
void FileTransmission::openFile()
{
    // Let user select a file to open
    auto path = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Select file to transmit"),
                                             QDir::homePath());

    // Filename is empty, abort
    if (path.isEmpty())
        return;

    // Close current file (if any)
    if (fileOpen())
        closeFile();

    // Try to open the file as read-only
    m_file.setFileName(path);
    if (m_file.open(QFile::ReadOnly))
    {
        m_stream = new QTextStream(&m_file);

        emit fileChanged();
        emit transmissionProgressChanged();
    }

    // Log open errors
    else
        qWarning() << "File open error" << m_file.errorString();
}

/**
 * Closes the currently selected file
 */
void FileTransmission::closeFile()
{
    // Stop transmitting the file to the serial device
    stopTransmission();

    // Close current file
    if (m_file.isOpen())
        m_file.close();

    // Reset text stream handler
    delete m_stream;

    // Emit signals to update the UI
    emit fileChanged();
    emit transmissionProgressChanged();
}

/**
 * Pauses the file transmission process
 */
void FileTransmission::stopTransmission()
{
    m_timer.stop();
    emit activeChanged();
}

/**
 * Starts/resumes the file transmission process.ç
 *
 * @note If the file was already transmitted to the serial device, calling
 *       this function shall restart the file transmission process.
 */
void FileTransmission::beginTransmission()
{
    // Only allow transmission if serial device is open
    if (Manager::getInstance()->connected())
    {
        // If file has already been sent, reset text stream position
        if (transmissionProgress() == 100)
        {
            m_stream->seek(0);
            emit transmissionProgressChanged();
        }

        // Start timer
        m_timer.start();
        emit activeChanged();
    }

    // Stop transmission if serial device is closed
    else
        stopTransmission();
}

/**
 * Changes the time interval to wait after sending one line from the
 * currently selected file.
 */
void FileTransmission::setLineTransmissionInterval(const int msec)
{
    m_timer.setInterval(qMax(0, msec));
}

/**
 * Transmits a new line from the selected file to the serial port device.
 *
 * @note If EOF is reached, then the transmission process is automatically
 *       stopped
 */
void FileTransmission::sendLine()
{
    // Transmission disabled, abort
    if (!active())
        return;

    // Serial device not open, abort
    if (!Manager::getInstance()->connected())
        return;

    // Send next line to serial port
    if (!m_stream->atEnd())
    {
        auto line = m_stream->readLine();
        if (!line.isEmpty())
        {
            if (!line.endsWith("\n"))
                line.append("\n");

            Manager::getInstance()->writeData(line.toUtf8());
            emit transmissionProgressChanged();
        }
    }

    // Reached end of file, stop transmission
    else
        stopTransmission();
}
