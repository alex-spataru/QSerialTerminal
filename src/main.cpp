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

#include <QtQml>
#include <QSysInfo>
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QQmlApplicationEngine>

#include <AppInfo.h>
#include <Misc/Utilities.h>
#include <Serial/Console.h>
#include <Serial/Manager.h>
#include <UI/TerminalWidget.h>

#ifdef Q_OS_WIN
#    include <windows.h>
#endif

/**
 * @brief Entry-point function of the application
 *
 * @param argc argument count
 * @param argv argument data
 *
 * @return qApp exit code
 */
int main(int argc, char **argv)
{
    // Fix console output on Windows (https://stackoverflow.com/a/41701133)
    // This code will only execute if the application is started from the comamnd prompt
#ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        // Open the console's active buffer
        (void)freopen("CONOUT$", "w", stdout);
        (void)freopen("CONOUT$", "w", stderr);

        // Force print new-line (to avoid printing text over user commands)
        printf("\n");
    }
#endif

    // Set application attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Init. application
    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName(APP_DEVELOPER);
    app.setOrganizationDomain(APP_SUPPORT_URL);
    app.setStyle(QStyleFactory::create("Fusion"));

    // Init application modules
    QQmlApplicationEngine engine;
    auto manager = Serial::Manager::getInstance();
    auto console = Serial::Console::getInstance();
    auto utilities = Misc::Utilities::getInstance();

    // Register custom QML properties
    qmlRegisterType<UI::TerminalWidget>("UI", 1, 0, "TerminalWidget");

    // Configure dark UI
    Misc::Utilities::configureDarkUi();

    // Init QML interface
    auto c = engine.rootContext();
    QQuickStyle::setStyle("Fusion");
    c->setContextProperty("Cpp_AppIcon", "qrc" APP_ICON);
    c->setContextProperty("Cpp_Serial_Manager", manager);
    c->setContextProperty("Cpp_Serial_Console", console);
    c->setContextProperty("Cpp_Misc_Utilities", utilities);
    c->setContextProperty("Cpp_AppName", app.applicationName());
    c->setContextProperty("Cpp_AppVersion", app.applicationVersion());
    c->setContextProperty("Cpp_AppOrganization", app.organizationName());
    c->setContextProperty("Cpp_AppOrganizationDomain", app.organizationDomain());
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // QML error, exit
    if (engine.rootObjects().isEmpty())
        return EXIT_FAILURE;

    // Enter application event loop
    return app.exec();
}
