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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import UI 1.0
import Qt.labs.settings 1.0

Control {
    id: root

    //
    // Signals
    //
    signal transmissionClicked()

    //
    // Custom properties
    //
    property alias vt100emulation: textEdit.vt100emulation
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Function to send through serial port data
    //
    function sendData() {
        Cpp_Serial_Console.send(send.text)
        send.clear()
    }

    //
    // Clears console output
    //
    function clearConsole() {
        Cpp_Serial_Console.clear()
        textEdit.clear()
    }

    //
    // Copy function
    //
    function copy() {
        textEdit.copy()
    }

    //
    // Select all text
    //
    function selectAll() {
        textEdit.selectAll()
    }

    //
    // Save settings
    //
    Settings {
        property alias autoscroll: autoscrollCheck.checked
        property alias vt100Enabled: textEdit.vt100emulation
        property alias lineEnding: lineEndingCombo.currentIndex
        property alias displayMode: displayModeCombo.currentIndex
    }

    //
    // Remove selection
    //
    Shortcut {
        sequence: "escape"
        onActivated: textEdit.clearSelection()
    }

    //
    // Right-click context menu
    //
    Menu {
        id: contextMenu

        MenuItem {
            id: copyMenu
            text: qsTr("Copy")
            opacity: enabled ? 1 : 0.5
            onClicked: textEdit.copy()
            enabled: textEdit.copyAvailable
        }

        MenuItem {
            text: qsTr("Select all")
            enabled: !textEdit.empty
            opacity: enabled ? 1 : 0.5
            onTriggered: textEdit.selectAll()
        }

        MenuItem {
            text: qsTr("Clear")
            opacity: enabled ? 1 : 0.5
            onTriggered: root.clearConsole()
            enabled: Cpp_Serial_Console.saveAvailable
        }

        MenuSeparator {}

        MenuItem {
            opacity: enabled ? 1 : 0.5
            text: qsTr("Print")
            enabled: Cpp_Serial_Console.saveAvailable
            onTriggered: Cpp_Serial_Console.print(app.monoFont)
        }

        MenuItem {
            opacity: enabled ? 1 : 0.5
            text: qsTr("Save as") + "..."
            onTriggered: Cpp_Serial_Console.save()
            enabled: Cpp_Serial_Console.saveAvailable
        }

        MenuSeparator {
            visible: app.menuBar !== null
        }

        MenuItem {
            enabled: visible
            visible: app.menuBar !== null
            onTriggered: app.toggleMenubar()
            height: visible ? implicitHeight : 0
            text: visible && app.menuBar.visible ? qsTr("Hide menubar") :
                                                   qsTr("Show menubar")
        }
    }

    //
    // Controls
    //
    ColumnLayout {
        anchors.fill: parent
        spacing: app.spacing
        anchors.margins: app.spacing * 1.5

        //
        // Console display
        //
        TerminalWidget {
            id: textEdit
            focus: true
            readOnly: true
            font.pixelSize: 12
            vt100emulation: true
            centerOnScroll: false
            undoRedoEnabled: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            maximumBlockCount: 12000
            palette.text: "#8ecd9d"
            palette.base: "#121218"
            palette.button: "#16232a"
            palette.window: "#0d1217"
            font.family: app.monoFont
            autoscroll: Cpp_Serial_Console.autoscroll
            wordWrapMode: Text.WrapAtWordBoundaryOrAnywhere
            placeholderText: qsTr("No data received so far") + "..."

            MouseArea {
                id: mouseArea
                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.IBeamCursor
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton
                anchors.rightMargin: textEdit.scrollbarWidth
                onContainsMouseChanged: {
                    if (mouseArea.containsMouse)
                        textEdit.forceActiveFocus()
                }

                onClicked: {
                    if (mouse.button == Qt.RightButton) {
                        contextMenu.popup()
                        mouse.accepted = true
                    }
                }
            }
        }

        //
        // Data-write controls
        //
        RowLayout {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: Cpp_Serial_Manager.connected
            Behavior on opacity {NumberAnimation{}}

            //
            // Send file button
            //
            Button {
                height: 24
                Layout.maximumWidth: 32
                icon.color: palette.text
                icon.source: "qrc:/icons/attach.svg"
                onClicked: root.transmissionClicked()
            }

            //
            // Send data textfield
            //
            TextField {
                id: send
                height: 24
                font: textEdit.font
                Layout.fillWidth: true
                palette.text: "#8ecd9d"
                palette.base: "#121218"
                placeholderText: qsTr("Send data to device") + "..."

                //
                // Validate hex strings
                //
                validator: RegExpValidator {
                    regExp: displayModeCombo.currentIndex === 1 ? /^(?:([a-f0-9]{2})\s*)+$/i : /[\s\S]*/
                }

                //
                // Send data on <enter>
                //
                Keys.onReturnPressed: root.sendData()

                //
                // Add space automatically in hex view
                //
                onTextChanged: {
                    if (displayModeCombo.currentIndex === 1)
                        send.text = Cpp_Serial_Console.formatUserHex(send.text)
                }

                //
                // Navigate command history upwards with <up>
                //
                Keys.onUpPressed: {
                    Cpp_Serial_Console.historyUp()
                    send.text = Cpp_Serial_Console.currentHistoryString
                }

                //
                // Navigate command history downwards with <down>
                //
                Keys.onDownPressed: {
                    Cpp_Serial_Console.historyDown()
                    send.text = Cpp_Serial_Console.currentHistoryString
                }
            }

            //
            // Send file button
            //
            Button {
                height: 24
                text: qsTr("Send")
                icon.color: palette.text
                onClicked: root.sendData()
                icon.source: "qrc:/icons/send.svg"
            }
        }

        //
        // Terminal output options
        //
        RowLayout {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: Cpp_Serial_Manager.connected
            Behavior on opacity {NumberAnimation{}}

            CheckBox {
                id: autoscrollCheck
                text: qsTr("Autoscroll")
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_Serial_Console.autoscroll
                onCheckedChanged: {
                    if (Cpp_Serial_Console.autoscroll != checked)
                        Cpp_Serial_Console.autoscroll = checked
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ComboBox {
                id: lineEndingCombo
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_Serial_Console.lineEndings()
                currentIndex: Cpp_Serial_Console.lineEnding
                onCurrentIndexChanged: {
                    if (currentIndex != Cpp_Serial_Console.lineEnding)
                        Cpp_Serial_Console.lineEnding = currentIndex
                }
            }

            ComboBox {
                id: displayModeCombo
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_Serial_Console.displayModes()
                currentIndex: Cpp_Serial_Console.displayMode
                onCurrentIndexChanged: {
                    if (currentIndex != Cpp_Serial_Console.displayMode) {
                        Cpp_Serial_Console.displayMode = currentIndex
                        Cpp_Serial_Console.dataMode = currentIndex
                    }
                }
            }

            Button {
                height: 24
                Layout.maximumWidth: 32
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                onClicked: Cpp_Serial_Console.save()
                icon.source: "qrc:/icons/save.svg"
                enabled: Cpp_Serial_Console.saveAvailable
                Behavior on opacity {NumberAnimation{}}
            }

            Button {
                height: 24
                Layout.maximumWidth: 32
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                onClicked: root.clearConsole()
                icon.source: "qrc:/icons/delete.svg"
                enabled: Cpp_Serial_Console.saveAvailable
                Behavior on opacity {NumberAnimation{}}
            }
        }
    }
}
