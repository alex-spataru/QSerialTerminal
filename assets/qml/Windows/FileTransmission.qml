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

import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Qt.labs.settings 1.0

Window {
    id: root

    //
    // Window options
    //
    title: qsTr("File transmission")
    minimumWidth: column.implicitWidth + 4 * app.spacing
    maximumWidth: column.implicitWidth + 4 * app.spacing
    minimumHeight: column.implicitHeight + 4 * app.spacing
    maximumHeight: column.implicitHeight + 4 * app.spacing
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

    //
    // Save settings
    //
    Settings {
        property alias interval: _interval.value
    }

    //
    // Use page item to set application palette
    //
    Page {
        anchors.margins: 0
        anchors.fill: parent
        palette.text: app.foregroundColor
        palette.buttonText: app.foregroundColor
        palette.windowText: app.foregroundColor
        palette.window: app.windowBackgroundColor

        //
        // Window controls
        //
        ColumnLayout {
            id: column
            spacing: app.spacing
            anchors.centerIn: parent

            //
            // File selection
            //
            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true

                Label {
                    text: qsTr("File selection") + ":"
                }

                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    TextField {
                        enabled: false
                        Layout.fillWidth: true
                        Layout.minimumWidth: 256
                        Layout.alignment: Qt.AlignVCenter
                        text: Cpp_Serial_FileTransmission.fileName
                    }

                    Button {
                        text: qsTr("Select File")
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: Cpp_Serial_FileTransmission.openFile()
                    }
                }
            }

            //
            // Interval selector
            //
            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                enabled: Cpp_Serial_FileTransmission.fileOpen
                Behavior on opacity {NumberAnimation{}}

                Label {
                    text: qsTr("Transmission interval") + ":"
                }

                RowLayout {
                    spacing: app.spacing

                    SpinBox {
                        id: _interval
                        from: 0
                        to: 10000
                        editable: true
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        value: Cpp_Serial_FileTransmission.lineTransmissionInterval
                        onValueChanged: {
                            if (value !== Cpp_Serial_FileTransmission.lineTransmissionInterval)
                                Cpp_Serial_FileTransmission.lineTransmissionInterval = value
                        }
                    }

                    Label {
                        text: qsTr("msecs")
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }

            //
            // Transmission progressbar + start/stop button
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                enabled: Cpp_Serial_FileTransmission.fileOpen
                Behavior on opacity {NumberAnimation{}}

                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    Label {
                        text: qsTr("Progress: %1").arg(Cpp_Serial_FileTransmission.transmissionProgress) + "%"
                    }

                    ProgressBar {
                        from: 0
                        to: 100
                        Layout.fillWidth: true
                        value: Cpp_Serial_FileTransmission.transmissionProgress
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignBottom
                    checked: Cpp_Serial_FileTransmission.active
                    text: (Cpp_Serial_FileTransmission.transmissionProgress > 0 &&
                           Cpp_Serial_FileTransmission.transmissionProgress < 100) ?
                              (Cpp_Serial_FileTransmission.active ? qsTr("Pause transmission") :
                                                                    qsTr("Resume transmission")) :
                              (Cpp_Serial_FileTransmission.active ? qsTr("Stop transmission") :
                                                                    qsTr("Begin transmission"))
                    onClicked: {
                        if (Cpp_Serial_FileTransmission.active)
                            Cpp_Serial_FileTransmission.stopTransmission()
                        else
                            Cpp_Serial_FileTransmission.beginTransmission()
                    }
                }
            }
        }
    }
}
