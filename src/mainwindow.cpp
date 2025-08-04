#include "mainwindow.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_globalHotkeyManager(new GlobalHotkeyManager(this)), m_audioRecorder(new AudioRecorder(this)), m_keyboardSimulator(new KeyboardSimulator())
{
    setupUI();
    setupConnections();
    loadSettings();
    setupSystemTray();

    setWindowTitle("Pineapple Writer");
    setFixedSize(450, 400);
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

    // OpenAI API Key Group
    apiGroupBox = new QGroupBox("OpenAI API Key", centralWidget);
    apiLayout = new QHBoxLayout(apiGroupBox);

    apiLabel = new QLabel("API Key:", apiGroupBox);
    apiKeyEdit = new QLineEdit(apiGroupBox);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setPlaceholderText("Enter your OpenAI API key");

    apiLayout->addWidget(apiLabel);
    apiLayout->addWidget(apiKeyEdit);

    // Hotkey Group
    hotkeyGroupBox = new QGroupBox("Global Hotkey", centralWidget);
    hotkeyLayout = new QVBoxLayout(hotkeyGroupBox);

    hotkeyLabel = new QLabel("Click the button below and press your desired key combination to toggle transcription:", hotkeyGroupBox);
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
    connect(m_globalHotkeyManager, &GlobalHotkeyManager::pttStateChanged, this, &MainWindow::onPttStateChanged);

    // Connect transcription signals
    connect(m_audioRecorder, &AudioRecorder::transcriptionReceived,
            this, &MainWindow::onTranscriptionReceived);
    connect(m_audioRecorder, &AudioRecorder::transcriptionError,
            this, &MainWindow::onTranscriptionError);
}

void MainWindow::onPttStateChanged(bool isActive)
{
    qDebug() << "PushToTalk state changed to:" << (isActive ? "Active" : "Inactive");

    if (isActive)
    {
        startTranscription();
    }
    else
    {
        stopTranscription();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    hide(); // Hide instead of closing
    event->ignore();
}

void MainWindow::saveSettings()
{
    QSettings settings("Pineapple Writer", "Pineapple Writer");
    settings.setValue("hotkey", hotkeyWidget->getHotkey());
}

void MainWindow::loadSettings()
{
    QSettings settings("Pineapple Writer", "Pineapple Writer");
    QString hotkey = settings.value("hotkey", "").toString();
    if (hotkey.isEmpty())
    {
        hotkey = "Ctrl+Alt+F";
    }

    hotkeyWidget->setHotkey(hotkey);
    registerGlobalHotkey(hotkey);

    loadApiKey();
}

void MainWindow::saveApiKey()
{
    QSettings settings("Pineapple Writer", "Pineapple Writer");
    settings.setValue("apiKey", apiKeyEdit->text());
}

void MainWindow::loadApiKey()
{
    QSettings settings("Pineapple Writer", "Pineapple Writer");
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

    // Toggle transcription
    if (m_audioRecorder->isTranscribing())
    {
        qDebug() << "Stopping transcription...";
        stopTranscription();
    }
    else
    {
        qDebug() << "Starting transcription...";
        QString apiKey = apiKeyEdit->text().trimmed();
        if (apiKey.isEmpty())
        {
            QMessageBox::warning(this, "API Key Required",
                                 "Please enter your OpenAI API key before starting transcription.");
            return;
        }

        startTranscription();
    }
}

void MainWindow::onTranscriptionReceived(const QString &text)
{
    qDebug() << "Transcription received:" << text;

    // Type the received text using keyboard simulation
    if (m_keyboardSimulator && m_keyboardSimulator->isAvailable())
    {
        bool success = m_keyboardSimulator->typeText(text);
        if (!success)
        {
            qWarning() << "Failed to type transcription";
        }
    }
    else
    {
        qWarning() << "Keyboard simulator not available - cannot type transcription";
    }
}

void MainWindow::onTranscriptionError(const QString &error)
{
    qWarning() << "Transcription error:" << error;
    QMessageBox::warning(this, "Transcription Error",
                         "Transcription error: " + error);
}

void MainWindow::startTranscription()
{
    QString apiKey = apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty())
    {
        QMessageBox::warning(this, "API Key Required",
                             "Please enter your OpenAI API key before starting transcription.");
        return;
    }

    m_audioRecorder->setOpenAIApiKey(apiKey);
    m_audioRecorder->startTranscription();
    m_keyboardSimulator->onStreamingStarted();

    // Update tray icon to show recording
    updateTrayIcon(true);
}

void MainWindow::stopTranscription()
{
    m_audioRecorder->stopTranscription();
    updateTrayIcon(false);
}

void MainWindow::setupSystemTray()
{
    // Create system tray icon
    m_trayIcon = new QSystemTrayIcon(this);

    // Load icon from resources
    m_defaultIcon = QPixmap(":/appicon.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_recordingIcon = createRecordingIcon();

    // Set initial icon
    m_trayIcon->setIcon(QIcon(m_defaultIcon));
    m_trayIcon->setToolTip("Pineapple Writer");

    // Create tray menu
    m_trayMenu = new QMenu(this);
    m_openAction = m_trayMenu->addAction("Open");
    m_quitAction = m_trayMenu->addAction("Quit");

    // Connect signals
    connect(m_openAction, &QAction::triggered, this, &MainWindow::show);
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason)
            {
        if (reason == QSystemTrayIcon::Trigger) {
            show();
            raise();
            activateWindow();
        } });

    // Set the context menu on the tray icon
    m_trayIcon->setContextMenu(m_trayMenu);

    // Show tray icon
    m_trayIcon->show();
}

void MainWindow::updateTrayIcon(bool isRecording)
{
    if (isRecording)
    {
        m_trayIcon->setIcon(QIcon(m_recordingIcon));
        m_trayIcon->setToolTip("Pineapple Writer - Recording");
    }
    else
    {
        m_trayIcon->setIcon(QIcon(m_defaultIcon));
        m_trayIcon->setToolTip("Pineapple Writer");
    }
}

QPixmap MainWindow::createRecordingIcon()
{
    // Create a copy of the default icon
    QPixmap icon = m_defaultIcon;

    // Create a painter to draw on the icon
    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw a green circle in the bottom-right corner
    QColor greenColor(0, 255, 0); // Bright green
    painter.setPen(Qt::NoPen);
    painter.setBrush(greenColor);

    // Calculate circle position and size
    int circleSize = icon.width() / 2; // 1/2 of icon size
    int margin = 2;
    int x = icon.width() - circleSize - margin;
    int y = icon.height() - circleSize - margin;

    painter.drawEllipse(x, y, circleSize, circleSize);

    painter.end();
    return icon;
}