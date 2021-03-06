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
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import Qt.labs.settings 1.0

ApplicationWindow {
    id: app

    //
    // 'Pointer' to UI object
    //
    property UI ui: null

    //
    // Global properties
    //
    readonly property int spacing: 8
    readonly property color foregroundColor: "#ffffff"
    readonly property color windowBackgroundColor: "#121920"
    readonly property string monoFont: {
        switch (Qt.platform.os) {
        case "osx":
            return "Menlo"
        case "windows":
            return "Consolas"
        default:
            return "Monospace"
        }
    }

    //
    // Hacks to fix window maximized behavior
    //
    property bool firstChange: true
    property bool windowMaximized: false
    onVisibilityChanged: {
        if (visibility == Window.Maximized) {
            if (!windowMaximized)
                firstChange = false

            windowMaximized = true
        }

        else if (visibility !== Window.Hidden) {
            if (windowMaximized && firstChange) {
                app.x = 100
                app.y = 100
                app.width = app.minimumWidth
                app.height = app.minimumHeight
            }

            windowMaximized = false
        }
    }

    //
    // Window geometry
    //
    visible: false
    minimumWidth: 800
    minimumHeight: 600
    title: Cpp_AppName + " v" + Cpp_AppVersion

    //
    // Theme options
    //
    palette.text: app.foregroundColor
    palette.buttonText: app.foregroundColor
    palette.windowText: app.foregroundColor
    palette.window: app.windowBackgroundColor

    //
    // Load window position from settings & show window
    //
    function displayWindow() {
        // Startup verifications to ensure that bad settings
        // do not make our app reside outside screen
        if (x < 0 || x >= Screen.desktopAvailableWidth)
            x = 100
        if (y < 0 || y >= Screen.desktopAvailableHeight)
            y = 100

        // Startup verifications to ensure that app fits in current screen
        if (width > Screen.desktopAvailableWidth) {
            x = 100
            width = Screen.desktopAvailableWidth - x
        }

        // Startup verifications to ensure that app fits in current screen
        if (height > Screen.desktopAvailableHeight) {
            y = 100
            height = Screen.desktopAvailableHeight - y
        }

        // Show app window
        if (app.windowMaximized)
            app.showMaximized()
        else
            app.showNormal()
    }

    //
    // Save window size & position
    //
    Settings {
        property alias appX: app.x
        property alias appY: app.y
        property alias appW: app.width
        property alias appH: app.height
        property alias appMaximized: app.windowMaximized
    }

    //
    // UI content loader
    //
    Loader {
        id: loader
        asynchronous: true
        anchors.fill: parent
        sourceComponent: UI {
            anchors.fill: parent
            Component.onCompleted: {
                app.ui = this
                app.displayWindow()
            }
        }
    }
}
