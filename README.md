<a href="#">
    <img width="192px" height="192px" src="doc/icon.svg" align="right" />
</a>

# QSerialTerminal

[![Build Status](https://github.com/alex-spataru/QSerialTerminal/workflows/Build/badge.svg)](https://github.com/alex-spataru/QSerialTerminal/actions)
[![Github All Releases](https://img.shields.io/github/downloads/alex-spataru/QSerialTerminal/total.svg)](https://github.com/alex-spataru/QSerialTerminal/releases/)

A simple, cross-platform serial port terminal application written with Qt/QML.

Features:
- Display data in text & hexadecimal format
- Line ending control
- Optional autoscrolling
- Export terminal output to file
- File transmission to serial port device

![Main window screenshot](doc/screenshot.png)

For more advanced features, and active development, take a look at [Serial Studio](https://github.com/serial-studio/serial-studio).

## Compiling the application

Once you have Qt installed, open *QSerialTerminal.pro* in Qt Creator and click the "Run" button.

Alternatively, you can also use the following commands:

	qmake
	make -j4

## Licence

This project is released under the terms and conditions of the [MIT License](LICENSE.md).
