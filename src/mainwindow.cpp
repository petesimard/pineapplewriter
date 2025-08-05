#include "mainwindow.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>
#include <QStyle>
#include "pushtotalk.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_globalHotkeyManager(new GlobalHotkeyManager(this)), m_audioRecorder(new AudioRecorder(this)),
      m_keyboardSimulator(new KeyboardSimulator()), m_openAITranscriber(new OpenAITranscriberPost(this))
{
    setupUI();
    setupConnections();
    loadSettings();
    setupSystemTray();

    setWindowTitle("Pineapple Writer");

    setFixedSize(470, 350);
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

    // Input Method Group
    inputMethodGroupBox = new QGroupBox("Input Method", centralWidget);
    inputMethodLayout = new QVBoxLayout(inputMethodGroupBox);

    // Radio buttons for input method
    inputMethodButtonGroup = new QButtonGroup(this);
    toggleModeRadio = new QRadioButton("Toggle Mode", inputMethodGroupBox);
    pttModeRadio = new QRadioButton("Push-to-Talk Mode", inputMethodGroupBox);

    inputMethodButtonGroup->addButton(toggleModeRadio, 0);
    inputMethodButtonGroup->addButton(pttModeRadio, 1);

    inputMethodLayout->addWidget(toggleModeRadio);
    inputMethodLayout->addWidget(pttModeRadio);

    pttGroupBox = new QGroupBox("Push-to-Talk", centralWidget);
    auto pttLayout = new QVBoxLayout(pttGroupBox);

    // PTT Key Selection
    pttKeyLabel = new QLabel("PTT Key:", pttGroupBox);
    pttKeyComboBox = new QComboBox(pttGroupBox);

    // Add available keys from pushtotalk.h
    pttKeyComboBox->addItem("Right Alt", Alt_R);
    pttKeyComboBox->addItem("Left Alt", Alt_L);
    pttKeyComboBox->addItem("Right Shift", Shift_R);
    pttKeyComboBox->addItem("Left Shift", Shift_L);
    pttKeyComboBox->addItem("Right Control", Control_R);
    pttKeyComboBox->addItem("Left Control", Control_L);
    pttKeyComboBox->addItem("Caps Lock", Caps_Lock);
    pttKeyComboBox->addItem("Right Meta", Meta_R);
    pttKeyComboBox->addItem("Left Meta", Meta_L);
    pttKeyComboBox->addItem("Right Super", Super_R);
    pttKeyComboBox->addItem("Left Super", Super_L);
    pttKeyComboBox->addItem("Right Hyper", Hyper_R);
    pttKeyComboBox->addItem("Left Hyper", Hyper_L);

    QHBoxLayout *pttKeyLayout = new QHBoxLayout();
    pttKeyLayout->addWidget(pttKeyLabel);
    pttKeyLayout->addWidget(pttKeyComboBox);
    pttLayout->addLayout(pttKeyLayout);

    // Hotkey Group
    hotkeyGroupBox = new QGroupBox("Global Hotkey", centralWidget);
    hotkeyLayout = new QVBoxLayout(hotkeyGroupBox);

    hotkeyLabel = new QLabel("Click the button below and press your desired key combination to toggle transcription:", hotkeyGroupBox);
    hotkeyWidget = new HotkeyWidget(hotkeyGroupBox);

    hotkeyLayout->addWidget(hotkeyLabel);
    hotkeyLayout->addWidget(hotkeyWidget);

    // Add widgets to main layout
    mainLayout->addWidget(apiGroupBox);
    mainLayout->addWidget(inputMethodGroupBox);
    mainLayout->addWidget(hotkeyGroupBox);
    mainLayout->addWidget(pttGroupBox);
    mainLayout->addStretch();
}

void MainWindow::setupConnections()
{
    connect(apiKeyEdit, &QLineEdit::textChanged, this, &MainWindow::saveApiKey);
    connect(hotkeyWidget, &HotkeyWidget::hotkeyChanged, this, &MainWindow::onHotkeyChanged);
    connect(m_globalHotkeyManager, &GlobalHotkeyManager::hotkeyPressed, this, &MainWindow::onGlobalHotkeyPressed);
    connect(m_globalHotkeyManager, &GlobalHotkeyManager::pttStateChanged, this, &MainWindow::onPttStateChanged);

    // Connect input method signals
    connect(inputMethodButtonGroup, SIGNAL(buttonClicked(QAbstractButton *)),
            this, SLOT(onInputMethodChanged()));
    connect(pttKeyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onPttKeyChanged()));

    // Connect transcription signals
    connect(m_audioRecorder, &AudioRecorder::transcriptionReceived,
            this, &MainWindow::onTranscriptionReceived);
    connect(m_audioRecorder, &AudioRecorder::transcriptionError,
            this, &MainWindow::onTranscriptionError);

    connect(m_openAITranscriber, &OpenAITranscriberPost::transcriptionReceived,
            this, &MainWindow::onTranscriptionReceived);
    connect(m_openAITranscriber, &OpenAITranscriberPost::transcriptionError,
            this, &MainWindow::onTranscriptionError);
    connect(m_openAITranscriber, &OpenAITranscriberPost::transcriptionFinished,
            this, &MainWindow::onTranscriptionFinished);
}

void MainWindow::onTranscriptionFinished()
{
    currentState = IDLE;
    updateTrayIcon();
}

void MainWindow::onPttStateChanged(bool isActive)
{
    qDebug() << "PushToTalk state changed to:" << (isActive ? "Active" : "Inactive");

    if (isActive)
    {
        if (currentState == IDLE)
        {
            startRecording();
        }
    }
    else
    {
        if (currentState == RECORDING)
        {
            stopRecording();
        }
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
    settings.setValue("inputMethod", inputMethodButtonGroup->checkedId());
    settings.setValue("pttKey", pttKeyComboBox->currentData().toInt());
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

    // Load input method settings
    int inputMethod = settings.value("inputMethod", 0).toInt();
    if (inputMethod == 1)
    {
        pttModeRadio->setChecked(true);
    }
    else
    {
        toggleModeRadio->setChecked(true);
    }

    // Load PTT key setting
    int pttKey = settings.value("pttKey", Alt_R).toInt();
    int index = pttKeyComboBox->findData(pttKey);
    if (index >= 0)
    {
        pttKeyComboBox->setCurrentIndex(index);
    }

    // Update UI and apply settings
    updateInputMethodUI();
    onInputMethodChanged();
    onPttKeyChanged();

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
    if (m_audioRecorder->isRecording())
    {
        qDebug() << "Stopping recording...";
        stopRecording();
    }
    else
    {
        qDebug() << "Starting recording...";
        QString apiKey = apiKeyEdit->text().trimmed();
        if (apiKey.isEmpty())
        {
            QMessageBox::warning(this, "API Key Required",
                                 "Please enter your OpenAI API key before starting transcription.");
            return;
        }

        startRecording();
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

void MainWindow::startRecording()
{
    QString apiKey = apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty())
    {
        QMessageBox::warning(this, "API Key Required",
                             "Please enter your OpenAI API key before starting transcription.");
        return;
    }

    m_audioRecorder->startRecording();

    currentState = RECORDING;
    // Update tray icon to show recording
    updateTrayIcon();
}

void MainWindow::stopRecording()
{
    m_audioRecorder->stopRecording();

    m_openAITranscriber->setApiKey(apiKeyEdit->text().trimmed());
    m_openAITranscriber->setAudioBuffer(m_audioRecorder->getAudioBuffer());
    m_openAITranscriber->transcribeAudio();

    currentState = PROCESSING;
    updateTrayIcon();
}

void MainWindow::setupSystemTray()
{
    // Create system tray icon
    m_trayIcon = new QSystemTrayIcon(this);

    // Load icon from resources
    m_defaultIcon = QPixmap(":/appicon.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QColor greenColor(0, 255, 0); // Bright green
    m_recordingIcon = createRecordingIcon(greenColor);

    QColor yellowColor(255, 255, 0); // Bright yellow
    m_processingIcon = createRecordingIcon(yellowColor);

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

void MainWindow::updateTrayIcon()
{
    if (currentState == RECORDING)
    {
        m_trayIcon->setIcon(QIcon(m_recordingIcon));
        m_trayIcon->setToolTip("Pineapple Writer - Recording");
    }
    else if (currentState == PROCESSING)
    {
        m_trayIcon->setIcon(QIcon(m_processingIcon));
        m_trayIcon->setToolTip("Pineapple Writer - Processing");
    }
    else
    {
        m_trayIcon->setIcon(QIcon(m_defaultIcon));
        m_trayIcon->setToolTip("Pineapple Writer");
    }
}

QPixmap MainWindow::createRecordingIcon(QColor color)
{
    // Create a copy of the default icon
    QPixmap icon = m_defaultIcon;

    // Create a painter to draw on the icon
    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw a green circle in the bottom-right corner
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    // Calculate circle position and size
    int circleSize = icon.width() / 2; // 1/2 of icon size
    int margin = 2;
    int x = icon.width() - circleSize - margin;
    int y = icon.height() - circleSize - margin;

    painter.drawEllipse(x, y, circleSize, circleSize);

    painter.end();
    return icon;
}

void MainWindow::onInputMethodChanged()
{
    saveSettings();
    updateInputMethodUI();

    // Update GlobalHotkeyManager input method
    if (pttModeRadio->isChecked())
    {
        m_globalHotkeyManager->inputMethod = GlobalHotkeyManager::InputMethod::PTT;
    }
    else
    {
        m_globalHotkeyManager->inputMethod = GlobalHotkeyManager::InputMethod::Toggle;
    }
}

void MainWindow::onPttKeyChanged()
{
    saveSettings();

    // Update PTT key in GlobalHotkeyManager
    int pttKey = pttKeyComboBox->currentData().toInt();
    m_globalHotkeyManager->setPttKey(pttKey);
}

void MainWindow::updateInputMethodUI()
{
    bool isPttMode = pttModeRadio->isChecked();

    // Show/hide hotkey widget based on mode
    hotkeyGroupBox->setVisible(!isPttMode);
    pttGroupBox->setVisible(isPttMode);
}