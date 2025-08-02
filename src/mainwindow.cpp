#include "mainwindow.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_globalHotkeyManager(new GlobalHotkeyManager(this)), m_audioRecorder(new AudioRecorder(this)), m_keyboardSimulator(new KeyboardSimulator())
{
    setupUI();
    setupConnections();
    loadSettings();

    setWindowTitle("System Tray App");
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

    // Transcription Group
    transcriptionGroupBox = new QGroupBox("Real-time Transcription", centralWidget);
    transcriptionLayout = new QVBoxLayout(transcriptionGroupBox);

    transcriptionStatusLabel = new QLabel("Status: Not Transcribing", transcriptionGroupBox);
    transcriptionStatusLabel->setStyleSheet("font-weight: bold; color: red;");

    transcriptionTextLabel = new QLabel("Transcription will appear here...", transcriptionGroupBox);
    transcriptionTextLabel->setWordWrap(true);
    transcriptionTextLabel->setMinimumHeight(60);
    transcriptionTextLabel->setStyleSheet("border: 1px solid #ccc; padding: 5px; background-color: #f9f9f9;");

    transcriptionButtonLayout = new QHBoxLayout();
    startTranscriptionButton = new QPushButton("Start Transcription", transcriptionGroupBox);
    stopTranscriptionButton = new QPushButton("Stop Transcription", transcriptionGroupBox);
    stopTranscriptionButton->setEnabled(false);

    transcriptionButtonLayout->addWidget(startTranscriptionButton);
    transcriptionButtonLayout->addWidget(stopTranscriptionButton);
    transcriptionButtonLayout->addStretch();

    transcriptionLayout->addWidget(transcriptionStatusLabel);
    transcriptionLayout->addWidget(transcriptionTextLabel);
    transcriptionLayout->addLayout(transcriptionButtonLayout);

    // Add widgets to main layout
    mainLayout->addWidget(apiGroupBox);
    mainLayout->addWidget(hotkeyGroupBox);
    mainLayout->addWidget(transcriptionGroupBox);
    mainLayout->addStretch();
}

void MainWindow::setupConnections()
{
    connect(apiKeyEdit, &QLineEdit::textChanged, this, &MainWindow::saveApiKey);
    connect(hotkeyWidget, &HotkeyWidget::hotkeyChanged, this, &MainWindow::onHotkeyChanged);
    connect(m_globalHotkeyManager, &GlobalHotkeyManager::hotkeyPressed, this, &MainWindow::onGlobalHotkeyPressed);

    // Connect transcription signals
    connect(m_audioRecorder, &AudioRecorder::transcriptionReceived,
            this, &MainWindow::onTranscriptionReceived);
    connect(m_audioRecorder, &AudioRecorder::transcriptionError,
            this, &MainWindow::onTranscriptionError);
    connect(startTranscriptionButton, &QPushButton::clicked,
            this, &MainWindow::onStartTranscriptionClicked);
    connect(stopTranscriptionButton, &QPushButton::clicked,
            this, &MainWindow::onStopTranscriptionClicked);
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

    // Toggle transcription
    if (m_audioRecorder->isTranscribing())
    {
        qDebug() << "Stopping transcription...";
        m_audioRecorder->stopTranscription();
        onStopTranscriptionClicked();
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
        m_audioRecorder->setOpenAIApiKey(apiKey);
        m_audioRecorder->startTranscription();
        onStartTranscriptionClicked();
    }

    // Show the window when hotkey is pressed
    show();
    raise();
    activateWindow();
}

void MainWindow::onTranscriptionReceived(const QString &text)
{
    // /qDebug() << "Transcription received:" << text;
    transcriptionTextLabel->setText(text);

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

void MainWindow::onStartTranscriptionClicked()
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

    transcriptionStatusLabel->setText("Status: Transcribing");
    transcriptionStatusLabel->setStyleSheet("font-weight: bold; color: green;");
    startTranscriptionButton->setEnabled(false);
    stopTranscriptionButton->setEnabled(true);
    transcriptionTextLabel->setText("Starting transcription...");
}

void MainWindow::onStopTranscriptionClicked()
{
    m_audioRecorder->stopTranscription();

    transcriptionStatusLabel->setText("Status: Not Transcribing");
    transcriptionStatusLabel->setStyleSheet("font-weight: bold; color: red;");
    startTranscriptionButton->setEnabled(true);
    stopTranscriptionButton->setEnabled(false);
    transcriptionTextLabel->setText("Transcription stopped.");
}