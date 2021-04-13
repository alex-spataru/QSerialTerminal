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

using namespace Serial;

static FileTransmission *INSTANCE = Q_NULLPTR;

FileTransmission::FileTransmission()
{
    m_stream = Q_NULLPTR;

    // Send a line to the serial device periodically
    m_timer.setInterval(10);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendLine);

    // Stop transmission if serial device is disconnected
    auto mgr = Manager::getInstance();
    connect(mgr, &Manager::closed, this, &FileTransmission::stopTransmission);
}

FileTransmission::~FileTransmission()
{
    closeFile();
}

FileTransmission *FileTransmission::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new FileTransmission;

    return INSTANCE;
}

bool FileTransmission::active() const
{
    return m_timer.isActive();
}

bool FileTransmission::fileOpen() const
{
    return m_file.isOpen();
}

QString FileTransmission::fileName() const
{
    if (!fileOpen())
        return tr("No file selected...");

    return QFileInfo(m_file).fileName();
}

QString FileTransmission::fileSize() const
{
    QString value;
    QString units;
    auto size = m_file.size();

    if (size < 1024)
    {
        value = QString::number(size);
        units = "bytes";
    }

    else if (size >= 1024 && size < 1024 * 1024)
    {
        double kb = static_cast<double>(size) / 1024.0;
        value = QString::number(kb, 'f', 2);
        units = "KB";
    }

    else
    {
        double mb = static_cast<double>(size) / (1024 * 1024.0);
        value = QString::number(mb, 'f', 2);
        units = "MB";
    }

    return QString("%1 %2").arg(value).arg(units);
}

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

int FileTransmission::lineTransmissionInterval() const
{
    return m_timer.interval();
}

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

void FileTransmission::stopTransmission()
{
    m_timer.stop();
    emit activeChanged();
}

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

void FileTransmission::setLineTransmissionInterval(const int msec)
{
    m_timer.setInterval(qMax(0, msec));
}

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
            Manager::getInstance()->writeData(line.toUtf8());
            emit transmissionProgressChanged();
        }
    }

    // Reached end of file, stop transmission
    else
        stopTransmission();
}
