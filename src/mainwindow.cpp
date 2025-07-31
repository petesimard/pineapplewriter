#include "mainwindow.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_globalHotkeyManager(new GlobalHotkeyManager(this))
{
    setupUI();
    setupConnections();
    loadSettings();

    setWindowTitle("System Tray App");
    setFixedSize(450, 300);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ElevenLabs API Key Group
    apiGroupBox = new QGroupBox("ElevenLabs API Key", centralWidget);
    apiLayout = new QHBoxLayout(apiGroupBox);

    apiLabel = new QLabel("API Key:", apiGroupBox);
    apiKeyEdit = new QLineEdit(apiGroupBox);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setPlaceholderText("Enter your ElevenLabs API key");

    apiLayout->addWidget(apiLabel);
    apiLayout->addWidget(apiKeyEdit);

    // Hotkey Group
    hotkeyGroupBox = new QGroupBox("Global Hotkey", centralWidget);
    hotkeyLayout = new QVBoxLayout(hotkeyGroupBox);

    hotkeyLabel = new QLabel("Click the button below and press your desired key combination:", hotkeyGroupBox);
    hotkeyWidget = new HotkeyWidget(hotkeyGroupBox);

    hotkeyLayout->addWidget(hotkeyLabel);
    hotkeyLayout->addWidget(hotkeyWidget);

    // Add widgets to main layout
    mainLayout->addWidget(apiGroupBox);
    mainLayout->addWidget(hotkeyGroupBox);
    mainLayout->addStretch();
}

void MainWindow::setupConnections()
{
    connect(apiKeyEdit, &QLineEdit::textChanged, this, &MainWindow::saveApiKey);
    connect(hotkeyWidget, &HotkeyWidget::hotkeyChanged, this, &MainWindow::onHotkeyChanged);
    connect(m_globalHotkeyManager, &GlobalHotkeyManager::hotkeyPressed, this, &MainWindow::onGlobalHotkeyPressed);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    hide(); // Hide instead of closing
    event->ignore();
}

void MainWindow::saveSettings()
{
    QSettings settings("SystemTrayApp", "SystemTrayApp");
    settings.setValue("hotkey", hotkeyWidget->getHotkey());
}

void MainWindow::loadSettings()
{
    QSettings settings("SystemTrayApp", "SystemTrayApp");
    QString hotkey = settings.value("hotkey", "").toString();
    if (!hotkey.isEmpty())
    {
        hotkeyWidget->setHotkey(hotkey);
        registerGlobalHotkey(hotkey);
    }

    loadApiKey();
}

void MainWindow::saveApiKey()
{
    QSettings settings("SystemTrayApp", "SystemTrayApp");
    settings.setValue("apiKey", apiKeyEdit->text());
}

void MainWindow::loadApiKey()
{
    QSettings settings("SystemTrayApp", "SystemTrayApp");
    QString apiKey = settings.value("apiKey", "").toString();
    if (!apiKey.isEmpty())
    {
        apiKeyEdit->setText(apiKey);
    }
}

void MainWindow::onHotkeyChanged(const QString &hotkey)
{
    saveSettings();
    registerGlobalHotkey(hotkey);
}

void MainWindow::registerGlobalHotkey(const QString &hotkey)
{
    if (hotkey.isEmpty())
    {
        m_globalHotkeyManager->unregisterHotkey();
        return;
    }

    bool success = m_globalHotkeyManager->registerHotkey(hotkey);
    if (success)
    {
        qDebug() << "Successfully registered global hotkey:" << hotkey;
    }
    else
    {
        qWarning() << "Failed to register global hotkey:" << hotkey;
        QMessageBox::warning(this, "Hotkey Registration Failed",
                             "Failed to register the global hotkey. The hotkey might be already in use by another application.");
    }
}

void MainWindow::onGlobalHotkeyPressed()
{
    qDebug() << "Global hotkey pressed!";

    // Show the window when hotkey is pressed
    show();
    raise();
    activateWindow();

    // You can add your custom functionality here
    // For example, start recording, toggle visibility, etc.
}