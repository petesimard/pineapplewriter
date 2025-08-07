#include "mainwindow.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>
#include <QStyle>
#include <QDesktopServices>
#include <QUrl>
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

    setFixedSize(500, 450);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowIcon(QIcon(":/appicon.png"));
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::showUniversalError(const QString &title, const QString &message)
{
    // Show the UI if it's hidden
    if (!isVisible())
    {
        show();
        raise();
        activateWindow();
    }

    // Show the error message
    QMessageBox::warning(this, title, message);
}

void MainWindow::showEvent(QShowEvent *event)
{
    // If recording and UI is shown, stop recording and cancel transcription
    if (currentState == RECORDING)
    {
        stopRecording();
        currentState = IDLE;
        updateTrayIcon();
    }

    QMainWindow::showEvent(event);
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Create tab widget
    tabWidget = new QTabWidget(centralWidget);
    mainLayout->addWidget(tabWidget);

    // Setup tabs
    setupSetupTab();
    setupAudioTab();
    setupAdvancedTab();
}

void MainWindow::setupSetupTab()
{
    setupTab = new QWidget();
    setupLayout = new QVBoxLayout(setupTab);
    setupLayout->setSpacing(20);
    setupLayout->setContentsMargins(20, 20, 20, 20);

    // OpenAI API Key Group
    apiGroupBox = new QGroupBox("OpenAI API Key", setupTab);
    apiLayout = new QVBoxLayout(apiGroupBox);

    apiKeyLayout = new QHBoxLayout();
    apiLabel = new QLabel("API Key:", apiGroupBox);
    apiKeyEdit = new QLineEdit(apiGroupBox);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setPlaceholderText("Enter your OpenAI API key");

    apiKeyLayout->addWidget(apiLabel);
    apiKeyLayout->addWidget(apiKeyEdit);

    // API Key Link
    apiKeyLink = new QLabel(apiGroupBox);
    apiKeyLink->setText("<a href=\"https://platform.openai.com/api-keys\">Get API Key</a>");
    apiKeyLink->setOpenExternalLinks(false);
    apiKeyLink->setTextFormat(Qt::RichText);
    apiKeyLink->setStyleSheet("QLabel { color: blue; text-decoration: underline; }");

    apiLayout->addLayout(apiKeyLayout);
    apiLayout->addWidget(apiKeyLink);

    // Input Method Group
    inputMethodGroupBox = new QGroupBox("Input Method", setupTab);
    inputMethodLayout = new QVBoxLayout(inputMethodGroupBox);

    // Radio buttons for input method
    inputMethodButtonGroup = new QButtonGroup(this);
    toggleModeRadio = new QRadioButton("Toggle Mode", inputMethodGroupBox);
    pttModeRadio = new QRadioButton("Push-to-Talk Mode", inputMethodGroupBox);

    inputMethodButtonGroup->addButton(toggleModeRadio, 0);
    inputMethodButtonGroup->addButton(pttModeRadio, 1);

    inputMethodLayout->addWidget(toggleModeRadio);
    inputMethodLayout->addWidget(pttModeRadio);

    pttGroupBox = new QGroupBox("Push-to-Talk", setupTab);
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
    hotkeyGroupBox = new QGroupBox("Global Hotkey", setupTab);
    hotkeyLayout = new QVBoxLayout(hotkeyGroupBox);

    hotkeyLabel = new QLabel("Press the button below and press your desired key combination:", hotkeyGroupBox);
    hotkeyWidget = new HotkeyWidget(hotkeyGroupBox);

    hotkeyLayout->addWidget(hotkeyLabel);
    hotkeyLayout->addWidget(hotkeyWidget);

    // Add widgets to setup layout
    setupLayout->addWidget(apiGroupBox);
    setupLayout->addWidget(inputMethodGroupBox);
    setupLayout->addWidget(hotkeyGroupBox);
    setupLayout->addWidget(pttGroupBox);
    setupLayout->addStretch();

    // Add setup tab to tab widget
    tabWidget->addTab(setupTab, "Setup");
}

void MainWindow::setupAudioTab()
{
    audioTab = new QWidget();
    audioLayout = new QVBoxLayout(audioTab);
    audioLayout->setSpacing(20);
    audioLayout->setContentsMargins(20, 20, 20, 20);

    // Volume Group
    volumeGroupBox = new QGroupBox("Recording Volume", audioTab);
    volumeLayout = new QVBoxLayout(volumeGroupBox);

    volumeLabel = new QLabel("Volume Level:", volumeGroupBox);
    volumeSlider = new QSlider(Qt::Horizontal, volumeGroupBox);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(80);
    volumeSlider->setTickPosition(QSlider::TicksBelow);
    volumeSlider->setTickInterval(10);

    volumeValueLabel = new QLabel("80%", volumeGroupBox);
    volumeValueLabel->setAlignment(Qt::AlignCenter);

    volumeLayout->addWidget(volumeLabel);
    volumeLayout->addWidget(volumeSlider);
    volumeLayout->addWidget(volumeValueLabel);

    // Input Device Selection
    inputDeviceGroupBox = new QGroupBox("Input Device", audioTab);
    inputDeviceLayout = new QVBoxLayout(inputDeviceGroupBox);

    inputDeviceLabel = new QLabel("Select Input Device:", inputDeviceGroupBox);
    inputDeviceComboBox = new QComboBox(inputDeviceGroupBox);

    // Populate the combo box with available audio input devices
    populateInputDevices();

    inputDeviceLayout->addWidget(inputDeviceLabel);
    inputDeviceLayout->addWidget(inputDeviceComboBox);

    // Add widgets to audio layout
    audioLayout->addWidget(volumeGroupBox);
    audioLayout->addWidget(inputDeviceGroupBox);
    audioLayout->addStretch();

    // Add audio tab to tab widget
    tabWidget->addTab(audioTab, "Audio");
}

void MainWindow::setupAdvancedTab()
{
    advancedTab = new QWidget();
    advancedLayout = new QVBoxLayout(advancedTab);
    advancedLayout->setSpacing(20);
    advancedLayout->setContentsMargins(20, 20, 20, 20);

    // Model Selection
    modelGroupBox = new QGroupBox("Model Selection", advancedTab);
    modelLayout = new QVBoxLayout(modelGroupBox);

    modelLabel = new QLabel("Transcription Model:", modelGroupBox);
    modelComboBox = new QComboBox(modelGroupBox);

    // Add available models
    modelComboBox->addItem("whisper-1 (Default)");
    modelComboBox->addItem("whisper-1-large-v3");
    modelComboBox->addItem("whisper-1-large-v2");

    modelLayout->addWidget(modelLabel);
    modelLayout->addWidget(modelComboBox);

    // System Prompt
    systemPromptGroupBox = new QGroupBox("System Prompt", advancedTab);
    systemPromptLayout = new QVBoxLayout(systemPromptGroupBox);

    systemPromptLabel = new QLabel("Custom system prompt for transcription:", systemPromptGroupBox);
    systemPromptEdit = new QTextEdit(systemPromptGroupBox);
    systemPromptEdit->setPlaceholderText("Enter a custom system prompt to guide the transcription...");
    systemPromptEdit->setMaximumHeight(100);

    systemPromptLayout->addWidget(systemPromptLabel);
    systemPromptLayout->addWidget(systemPromptEdit);

    // Add widgets to advanced layout
    advancedLayout->addWidget(modelGroupBox);
    advancedLayout->addWidget(systemPromptGroupBox);
    advancedLayout->addStretch();

    // Add advanced tab to tab widget
    tabWidget->addTab(advancedTab, "Advanced");
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

    // Connect API key link
    connect(apiKeyLink, &QLabel::linkActivated, this, &MainWindow::onApiKeyLinkClicked);

    // Connect audio tab controls
    connect(volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);

    // Connect advanced tab controls
    connect(modelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModelChanged);
    connect(systemPromptEdit, &QTextEdit::textChanged, this, &MainWindow::onSystemPromptChanged);

    // Connect the combo box signal to handle device changes
    connect(inputDeviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onInputDeviceChanged);
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

    // Save input device selection
    if (inputDeviceComboBox->currentIndex() >= 0)
    {
        QVariant deviceId = inputDeviceComboBox->currentData();
        if (deviceId.isValid())
        {
            settings.setValue("inputDevice", deviceId.toString());
        }
    }
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
    int inputMethod = settings.value("inputMethod", GlobalHotkeyManager::InputMethod::PTT).toInt();
    if (inputMethod == 1)
    {
        pttModeRadio->setChecked(true);
    }
    else
    {
        toggleModeRadio->setChecked(true);
    }

    // Load PTT key setting - default to Right Alt
    int pttKey = settings.value("pttKey", Alt_R).toInt();
    int index = pttKeyComboBox->findData(pttKey);
    if (index >= 0)
    {
        pttKeyComboBox->setCurrentIndex(index);
    }

    // Load input device setting
    QString savedDeviceId = settings.value("inputDevice", "").toString();
    bool deviceSet = false;
    if (!savedDeviceId.isEmpty())
    {
        // Find the saved device in the combo box
        deviceSet = setAudioDeviceById(savedDeviceId);
    }

    if (!deviceSet)
    {
        // If no device is set, use the first available device
        QAudioDevice currentAudioDevice = m_audioRecorder->getCurrentAudioDevice();
        if (!currentAudioDevice.isNull())
        {
            QString currentDeviceId = currentAudioDevice.id();
            deviceSet = setAudioDeviceById(currentDeviceId);
        }
    }

    onInputMethodChanged();
    onPttKeyChanged();

    loadApiKey();
}

bool MainWindow::setAudioDeviceById(QString &savedDeviceId)
{
    int deviceIndex = inputDeviceComboBox->findData(savedDeviceId, 256, Qt::MatchContains);

    if (deviceIndex >= 0)
    {
        inputDeviceComboBox->setCurrentIndex(deviceIndex);

        // Apply the device to the audio recorder
        if (m_audioRecorder)
        {
            const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
            for (const QAudioDevice &device : inputDevices)
            {
                if (device.id() == savedDeviceId)
                {
                    setAudioDevice(device);
                    return true;
                }
            }
        }
    }

    return false;
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
        showUniversalError("Hotkey Registration Failed", "Failed to register the global hotkey. The hotkey might be already in use by another application.");
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
            showUniversalError("API Key Required", "Please enter your OpenAI API key.");
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
    showUniversalError("Transcription Error", "Transcription error: " + error);
}

void MainWindow::startRecording()
{
    // Disable recording if the UI is visible
    if (isVisible())
    {
        return;
    }

    QString apiKey = apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty())
    {
        showUniversalError("API Key Required", "Please enter your OpenAI API key before starting transcription.");
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

void MainWindow::onApiKeyLinkClicked()
{
    QDesktopServices::openUrl(QUrl("https://platform.openai.com/api-keys"));
}

void MainWindow::onVolumeChanged(int value)
{
    volumeValueLabel->setText(QString("%1%").arg(value));

    // Convert percentage (0-100) to volume (0.0-1.0)
    qreal volume = value / 100.0;

    // Apply volume setting to audio recorder
    if (m_audioRecorder)
    {
        m_audioRecorder->setVolume(volume);
    }

    // Save volume setting
    QSettings settings("Pineapple Writer", "Pineapple Writer");
    settings.setValue("volume", value);
}

void MainWindow::onInputDeviceChanged(int index)
{
    if (index >= 0 && index < inputDeviceComboBox->count())
    {
        QVariant deviceId = inputDeviceComboBox->itemData(index);
        if (deviceId.isValid())
        {
            // Find the device by ID
            const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
            for (const QAudioDevice &device : inputDevices)
            {
                if (device.id() == deviceId.toString())
                {
                    // Apply the selected input device to the audio recorder
                    if (m_audioRecorder)
                    {
                        setAudioDevice(device);
                    }

                    qDebug() << "Selected input device:" << inputDeviceComboBox->currentText()
                             << "with ID:" << deviceId.toString();

                    // Save device selection
                    QSettings settings("Pineapple Writer", "Pineapple Writer");
                    settings.setValue("inputDevice", deviceId.toString());
                    break;
                }
            }
        }
    }
}

void MainWindow::setAudioDevice(const QAudioDevice &device)
{
    m_audioRecorder->setAudioDevice(device);
    // Update volume display to match current device volume
    int currentVolume = m_audioRecorder->getVolume() * 100;
    volumeSlider->setValue(currentVolume);
    volumeValueLabel->setText(QString("%1%").arg(currentVolume));
}

void MainWindow::onModelChanged(int index)
{
    // TODO: Apply model selection to transcriber
    Q_UNUSED(index)
}

void MainWindow::onSystemPromptChanged()
{
    // TODO: Apply system prompt to transcriber
}

void MainWindow::updateInputMethodUI()
{
    bool isPttMode = pttModeRadio->isChecked();

    // Show/hide hotkey widget based on mode
    hotkeyGroupBox->setVisible(!isPttMode);
    pttGroupBox->setVisible(isPttMode);
}

void MainWindow::populateInputDevices()
{
    // Clear existing items
    inputDeviceComboBox->clear();

    // Get all available audio input devices
    const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();

    if (inputDevices.isEmpty())
    {
        inputDeviceComboBox->addItem("No input devices found");
        return;
    }

    // Add each input device to the combo box
    for (const QAudioDevice &device : inputDevices)
    {
        inputDeviceComboBox->addItem(device.description(), device.id());
    }
}