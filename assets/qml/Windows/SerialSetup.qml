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
    title: qsTr("Serial Setup")
    minimumWidth: column.implicitWidth + 4 * app.spacing
    maximumWidth: column.implicitWidth + 4 * app.spacing
    minimumHeight: column.implicitHeight + 4 * app.spacing
    maximumHeight: column.implicitHeight + 4 * app.spacing
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

    //
    // Save settings between runs
    //
    Settings {
        category: "serial"
        property alias port: _portCombo.currentIndex
        property alias baudRate: _baudCombo.currentIndex
        property alias dataBits: _dataCombo.currentIndex
        property alias parity: _parityCombo.currentIndex
        property alias flowControl: _flowCombo.currentIndex
        property alias stopBits: _stopBitsCombo.currentIndex
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
            // Reader information
            //
            GridLayout {
                id: layout
                columns: 2
                Layout.fillWidth: true
                rowSpacing: app.spacing
                columnSpacing: app.spacing

                //
                // Serial port selector
                //
                Label {
                    text: qsTr("Port") + ":"
                } ComboBox {
                    id: _portCombo
                    Layout.fillWidth: true
                    Layout.minimumWidth: 256
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
                } ComboBox {
                    id: _baudCombo
                    Layout.fillWidth: true
                    Layout.minimumWidth: 256
                    model: Cpp_Serial_Manager.baudRateList
                    currentIndex: Cpp_Serial_Manager.baudRateIndex
                    onCurrentIndexChanged: {
                        if (currentIndex !== Cpp_Serial_Manager.baudRateIndex)
                            Cpp_Serial_Manager.baudRateIndex = currentIndex
                    }
                }

                //
                // Data bits selector
                //
                Label {
                    text: qsTr("Data Bits") + ":"
                } ComboBox {
                    id: _dataCombo
                    Layout.fillWidth: true
                    model: Cpp_Serial_Manager.dataBitsList
                    currentIndex: Cpp_Serial_Manager.dataBitsIndex
                    onCurrentIndexChanged: {
                        if (Cpp_Serial_Manager.dataBitsIndex !== currentIndex)
                            Cpp_Serial_Manager.dataBitsIndex = currentIndex
                    }
                }

                //
                // Parity selector
                //
                Label {
                    text: qsTr("Parity") + ":"
                } ComboBox {
                    id: _parityCombo
                    Layout.fillWidth: true
                    model: Cpp_Serial_Manager.parityList
                    currentIndex: Cpp_Serial_Manager.parityIndex
                    onCurrentIndexChanged: {
                        if (Cpp_Serial_Manager.parityIndex !== currentIndex)
                            Cpp_Serial_Manager.parityIndex = currentIndex
                    }
                }

                //
                // Stop bits selector
                //
                Label {
                    text: qsTr("Stop Bits") + ":"
                } ComboBox {
                    id: _stopBitsCombo
                    Layout.fillWidth: true
                    model: Cpp_Serial_Manager.stopBitsList
                    currentIndex: Cpp_Serial_Manager.stopBitsIndex
                    onCurrentIndexChanged: {
                        if (Cpp_Serial_Manager.stopBitsIndex !== currentIndex)
                            Cpp_Serial_Manager.stopBitsIndex = currentIndex
                    }
                }

                //
                // Flow control selector
                //
                Label {
                    text: qsTr("Flow Control") + ":"
                } ComboBox {
                    id: _flowCombo
                    Layout.fillWidth: true
                    model: Cpp_Serial_Manager.flowControlList
                    currentIndex: Cpp_Serial_Manager.flowControlIndex
                    onCurrentIndexChanged: {
                        if (Cpp_Serial_Manager.flowControlIndex !== currentIndex)
                            Cpp_Serial_Manager.flowControlIndex = currentIndex
                    }
                }
            }
        }
    }
}
