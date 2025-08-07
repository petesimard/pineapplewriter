#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QPixmap>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QTabWidget>
#include <QSlider>
#include <QProgressBar>
#include <QTextEdit>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QDebug>
#include "hotkeywidget.h"
#include "globalhotkeymanager.h"
#include "audiorecorder.h"
#include "keyboardsimulator.h"
#include "openaitranscriber_post.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Universal error function
    void showUniversalError(const QString &title, const QString &message);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void saveSettings();
    void loadSettings();
    bool setAudioDeviceById(QString &savedDeviceId);
    void saveApiKey();
    void loadApiKey();
    void onHotkeyChanged(const QString &hotkey);
    void onGlobalHotkeyPressed();
    void onTranscriptionReceived(const QString &text);
    void onTranscriptionError(const QString &error);
    void onTranscriptionFinished();
    void startRecording();
    void stopRecording();
    void onPttStateChanged(bool isActive);
    void onInputMethodChanged();
    void onPttKeyChanged();
    void onApiKeyLinkClicked();
    void onVolumeChanged(int value);
    void onInputDeviceChanged(int index);
    void setAudioDevice(const QAudioDevice &device);
    void onModelChanged(int index);
    void onSystemPromptChanged();

private:
    void setupUI();
    void setupConnections();
    void registerGlobalHotkey(const QString &hotkey);
    void updateInputMethodUI();
    void setupSetupTab();
    void setupAudioTab();
    void setupAdvancedTab();
    void populateInputDevices();

    enum State
    {
        IDLE,
        RECORDING,
        PROCESSING
    };

    State currentState = IDLE;
    bool isLoadingSettings = false;

    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;

    // Setup Tab
    QWidget *setupTab;
    QVBoxLayout *setupLayout;
    QGroupBox *apiGroupBox;
    QVBoxLayout *apiLayout;
    QHBoxLayout *apiKeyLayout;
    QLabel *apiLabel;
    QLineEdit *apiKeyEdit;
    QLabel *apiKeyLink;

    QGroupBox *hotkeyGroupBox;
    QGroupBox *pttGroupBox;
    QVBoxLayout *hotkeyLayout;
    QLabel *hotkeyLabel;
    HotkeyWidget *hotkeyWidget;

    // Input method selection
    QGroupBox *inputMethodGroupBox;
    QVBoxLayout *inputMethodLayout;
    QButtonGroup *inputMethodButtonGroup;
    QRadioButton *toggleModeRadio;
    QRadioButton *pttModeRadio;
    QComboBox *pttKeyComboBox;
    QLabel *pttKeyLabel;

    // Audio Tab
    QWidget *audioTab;
    QVBoxLayout *audioLayout;
    QGroupBox *volumeGroupBox;
    QVBoxLayout *volumeLayout;
    QLabel *volumeLabel;
    QSlider *volumeSlider;
    QLabel *volumeValueLabel;

    QGroupBox *inputDeviceGroupBox;
    QVBoxLayout *inputDeviceLayout;
    QLabel *inputDeviceLabel;
    QComboBox *inputDeviceComboBox;

    // Advanced Tab
    QWidget *advancedTab;
    QVBoxLayout *advancedLayout;
    QGroupBox *modelGroupBox;
    QVBoxLayout *modelLayout;
    QLabel *modelLabel;
    QComboBox *modelComboBox;

    QGroupBox *systemPromptGroupBox;
    QVBoxLayout *systemPromptLayout;
    QLabel *systemPromptLabel;
    QTextEdit *systemPromptEdit;

    // Global hotkey manager
    GlobalHotkeyManager *m_globalHotkeyManager;

    // Audio recorder
    AudioRecorder *m_audioRecorder;

    // OpenAI transcriber
    OpenAITranscriberPost *m_openAITranscriber;

    // Keyboard simulator
    KeyboardSimulator *m_keyboardSimulator;

    // System tray
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_openAction;
    QAction *m_quitAction;
    QPixmap m_defaultIcon;
    QPixmap m_recordingIcon;
    QPixmap m_processingIcon;

    void setupSystemTray();
    void updateTrayIcon();
    QPixmap createRecordingIcon(QColor color);
};

#endif // MAINWINDOW_H