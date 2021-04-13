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
#include <QTextStream>

namespace Serial
{
class FileTransmission : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(bool fileOpen
               READ fileOpen
               NOTIFY fileChanged)
    Q_PROPERTY(bool active
               READ active
               NOTIFY activeChanged)
    Q_PROPERTY(QString fileName
               READ fileName
               NOTIFY fileChanged)
    Q_PROPERTY(int transmissionProgress
               READ transmissionProgress
               NOTIFY transmissionProgressChanged)
    Q_PROPERTY(int lineTransmissionInterval
               READ lineTransmissionInterval
               WRITE setLineTransmissionInterval
               NOTIFY lineTransmissionIntervalChanged)
    // clang-format on

signals:
    void fileChanged();
    void activeChanged();
    void transmissionProgressChanged();
    void lineTransmissionIntervalChanged();

public:
    static FileTransmission *getInstance();

    bool active() const;
    bool fileOpen() const;
    QString fileName() const;
    int transmissionProgress() const;
    int lineTransmissionInterval() const;

public slots:
    void openFile();
    void closeFile();
    void stopTransmission();
    void beginTransmission();
    void setLineTransmissionInterval(const int msec);

private slots:
    void sendLine();

private:
    FileTransmission();
    ~FileTransmission();

private:
    QFile m_file;
    QTimer m_timer;
    QTextStream *m_stream;
};
}

#endif
