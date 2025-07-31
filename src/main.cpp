#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
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

    // Create system tray icon
    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(app.style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon.setToolTip("System Tray App");

    // Create tray menu
    QMenu trayMenu;
    QAction *openAction = trayMenu.addAction("Open");
    QAction *quitAction = trayMenu.addAction("Quit");

    // Connect signals
    QObject::connect(openAction, &QAction::triggered, &window, &MainWindow::show);
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [&window](QSystemTrayIcon::ActivationReason reason)
                     {
        if (reason == QSystemTrayIcon::Trigger) {
            window.show();
            window.raise();
            window.activateWindow();
        } });

    // Set the context menu on the tray icon
    trayIcon.setContextMenu(&trayMenu);

    // Show tray icon
    trayIcon.show();

    return app.exec();
}