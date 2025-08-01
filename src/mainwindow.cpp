#include "mainwindow.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_globalHotkeyManager(new GlobalHotkeyManager(this)), m_audioRecorder(new AudioRecorder(this))
{
    setupUI();
    setupConnections();
    loadSettings();

    setWindowTitle("System Tray App");
    setFixedSize(450, 500);
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

    hotkeyLabel = new QLabel("Click the button below and press your desired key combination:", hotkeyGroupBox);
    hotkeyWidget = new HotkeyWidget(hotkeyGroupBox);

    hotkeyLayout->addWidget(hotkeyLabel);
    hotkeyLayout->addWidget(hotkeyWidget);

    // Recording Status Group
    recordingGroupBox = new QGroupBox("Audio Recording", centralWidget);
    recordingLayout = new QVBoxLayout(recordingGroupBox);

    recordingStatusLabel = new QLabel("Status: Not Recording", recordingGroupBox);
    recordingStatusLabel->setStyleSheet("font-weight: bold; color: red;");

    recordingInfoLabel = new QLabel("Press the hotkey to start/stop recording", recordingGroupBox);
    recordingInfoLabel->setWordWrap(true);

    // Buffer Size Control
    bufferSizeLayout = new QHBoxLayout();
    bufferSizeLabel = new QLabel("Buffer Size (MB):", recordingGroupBox);
    bufferSizeSpinBox = new QSpinBox(recordingGroupBox);
    bufferSizeSpinBox->setRange(1, 100);
    bufferSizeSpinBox->setValue(5);
    bufferSizeSpinBox->setSuffix(" MB");

    bufferSizeLayout->addWidget(bufferSizeLabel);
    bufferSizeLayout->addWidget(bufferSizeSpinBox);
    bufferSizeLayout->addStretch();

    recordingLayout->addWidget(recordingStatusLabel);
    recordingLayout->addWidget(recordingInfoLabel);
    recordingLayout->addLayout(bufferSizeLayout);

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
    mainLayout->addWidget(recordingGroupBox);
    mainLayout->addWidget(transcriptionGroupBox);
    mainLayout->addStretch();
}

void MainWindow::setupConnections()
{
    connect(apiKeyEdit, &QLineEdit::textChanged, this, &MainWindow::saveApiKey);
    connect(hotkeyWidget, &HotkeyWidget::hotkeyChanged, this, &MainWindow::onHotkeyChanged);
    connect(m_globalHotkeyManager, &GlobalHotkeyManager::hotkeyPressed, this, &MainWindow::onGlobalHotkeyPressed);

    // Connect audio recorder signals
    connect(m_audioRecorder, &AudioRecorder::recordingStarted, this, &MainWindow::onRecordingStarted);
    connect(m_audioRecorder, &AudioRecorder::recordingStopped, this, &MainWindow::onRecordingStopped);
    connect(m_audioRecorder, &AudioRecorder::recordingError, this, &MainWindow::onRecordingError);

    // Connect buffer size control
    connect(bufferSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onBufferSizeChanged);

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

    // Toggle audio recording
    if (m_audioRecorder->isRecording())
    {
        qDebug() << "Stopping audio recording...";
        m_audioRecorder->stopRecording();
    }
    else
    {
        qDebug() << "Starting audio recording...";
        m_audioRecorder->startRecording();
    }

    // Show the window when hotkey is pressed
    show();
    raise();
    activateWindow();
}

void MainWindow::onRecordingStarted()
{
    qDebug() << "Audio recording started";
    recordingStatusLabel->setText("Status: Recording");
    recordingStatusLabel->setStyleSheet("font-weight: bold; color: green;");
    recordingInfoLabel->setText("Recording audio... Press hotkey again to stop");
}

void MainWindow::onRecordingStopped()
{
    qDebug() << "Audio recording stopped";
    QByteArray audioData = m_audioRecorder->getRecordedAudio();
    qDebug() << "Recorded audio size:" << audioData.size() << "bytes";

    recordingStatusLabel->setText("Status: Not Recording");
    recordingStatusLabel->setStyleSheet("font-weight: bold; color: red;");
    recordingInfoLabel->setText(QString("Recording stopped. Captured %1 bytes of audio data. Press hotkey to start new recording.").arg(audioData.size()));
}

void MainWindow::onRecordingError(const QString &error)
{
    qWarning() << "Audio recording error:" << error;
    QMessageBox::warning(this, "Recording Error",
                         "Failed to record audio: " + error);
}

void MainWindow::onBufferSizeChanged(int size)
{
    // Convert MB to bytes
    int sizeInBytes = size * 1024 * 1024;
    m_audioRecorder->setBufferSize(sizeInBytes);
    qDebug() << "Buffer size changed to" << size << "MB (" << sizeInBytes << "bytes)";
}

void MainWindow::onTranscriptionReceived(const QString &text)
{
    qDebug() << "Transcription received:" << text;
    transcriptionTextLabel->setText(text);
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