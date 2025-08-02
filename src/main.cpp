#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false); // Keep app running when window is closed

    // Check if system tray is available
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(nullptr, "Error", "System tray is not available on this system.");
        return 1;
    }

    // Create main window
    MainWindow window;

    return app.exec();
}