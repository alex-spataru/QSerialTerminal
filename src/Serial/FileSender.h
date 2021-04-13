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

#ifndef SERIAL_FILE_SENDER_H
#define SERIAL_FILE_SENDER_H

#include <QFile>
#include <QTimer>
#include <QObject>

namespace Serial
{
class FileSender : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(bool fileOpen
               READ fileOpen
               NOTIFY fileChanged)
    Q_PROPERTY(QString fileName
               READ fileName
               NOTIFY fileChanged)
    Q_PROPERTY(QString fileSize
               READ fileSize
               NOTIFY fileChanged)
    Q_PROPERTY(int transmissionProgress
               READ transmissionProgress
               NOTIFY transmissionProgressChanged)
    Q_PROPERTY(int lineTransmissionInterval
               READ lineTransmissionInterval
               NOTIFY lineTransmissionIntervalChanged)
    // clang-format on

signals:
    void fileChanged();
    void transmissionProgressChanged();
    void lineTransmissionIntervalChanged();

public:
    static FileSender *getInstance();

    bool fileOpen() const;
    QString fileName() const;
    QString fileSize() const;
    int transmissionProgress() const;
    int lineTransmissionInterval() const;

public slots:
    void openFile();
    void closeFile();
    void stopTransmission();
    void beginTransmission();
    void setLineTransmissionInterval(const int interval);

private slots:
    void sendLine();

private:
    FileSender();
    ~FileSender();

private:
    QFile m_file;
    QTimer m_timer;
    quint64 m_sentBytes;
    int m_transmissionMode;
};
}

#endif
