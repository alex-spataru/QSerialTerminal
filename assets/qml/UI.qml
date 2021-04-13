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
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "Windows" as Windows
import "Widgets" as Widgets

Page {
    id: root

    //
    // Toolbar with buttons
    //
    header: ToolBar {
        height: 48

        //
        // Background gradient
        //
        Rectangle {
            border.width: 1
            border.color: palette.midlight

            gradient: Gradient {
                GradientStop { position: 0; color: "#21373f" }
                GradientStop { position: 1; color: "#11272f" }
            }

            anchors {
                fill: parent
                topMargin: -border.width
                leftMargin: -border.width * 10
                rightMargin: -border.width * 10
            }
        }

        //
        // Toolbar controls
        //
        RowLayout {
            spacing: app.spacing
            anchors.fill: parent
            anchors.margins: app.spacing

            //
            // Serial port selector
            //
            Label {
                text: qsTr("Port") + ":"
                Layout.alignment: Qt.AlignVCenter
            } ComboBox {
                id: _portCombo
                Layout.minimumWidth: 156
                Layout.maximumWidth: 156
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_Serial_Manager.portList
                currentIndex: Cpp_Serial_Manager.portIndex
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_Serial_Manager.portIndex)
                        Cpp_Serial_Manager.portIndex = currentIndex
                }
            }

            //
            // Baud rate selector
            //
            Label {
                text: qsTr("Baud rate") + ":"
                Layout.alignment: Qt.AlignVCenter
            } ComboBox {
                id: _baudCombo
                Layout.minimumWidth: 128
                Layout.maximumWidth: 128
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_Serial_Manager.baudRateList
                currentIndex: Cpp_Serial_Manager.baudRateIndex
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_Serial_Manager.baudRateIndex)
                        Cpp_Serial_Manager.baudRateIndex = currentIndex
                }
            }

            //
            // Horizontal spacing
            //
            Item {
                Layout.fillWidth: true
            }

            //
            // Serial setup button
            //
            Button {
                flat: true
                icon.width: 24
                icon.height: 24
                text: qsTr("Setup") + " "
                icon.color: palette.buttonText
                icon.source: "qrc:/icons/usb.svg"
                Layout.alignment: Qt.AlignVCenter
                onClicked: _serialSetup.showNormal()
            }

            //
            // Connect/disconnect button
            //
            Button {
                flat: true
                icon.width: 24
                icon.height: 24
                font.bold: true
                opacity: enabled ? 1 : 0.5
                icon.color: palette.buttonText
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_Serial_Manager.connected
                enabled: Cpp_Serial_Manager.configurationOk
                onClicked: Cpp_Serial_Manager.toggleConnection()
                palette.buttonText: checked ? "#d72d60" : "#2eed5c"
                text: (checked ? qsTr("Disconnect") : qsTr("Connect")) + " "
                icon.source: checked ? "qrc:/icons/disconnect.svg" : "qrc:/icons/connect.svg"

                Behavior on opacity {NumberAnimation{}}
            }
        }
    }

    //
    // Background color
    //
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Terminal widget
    //
    Widgets.Terminal {
        id: _terminal
        anchors.fill: parent
        onTransmissionClicked: _fileTransmission.showNormal()
    }

    //
    // Serial setup dialog
    //
    Windows.SerialSetup {
        id: _serialSetup
    }

    //
    // File transmission dialog
    //
    Windows.FileTransmission {
        id: _fileTransmission
    }
}
