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
#include "hotkeywidget.h"
#include "globalhotkeymanager.h"
#include "audiorecorder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void saveSettings();
    void loadSettings();
    void saveApiKey();
    void loadApiKey();
    void onHotkeyChanged(const QString &hotkey);
    void onGlobalHotkeyPressed();
    void onRecordingStarted();
    void onRecordingStopped();
    void onRecordingError(const QString &error);
    void onBufferSizeChanged(int size);
    void onTranscriptionReceived(const QString &text);
    void onTranscriptionError(const QString &error);
    void onStartTranscriptionClicked();
    void onStopTranscriptionClicked();

private:
    void setupUI();
    void setupConnections();
    void registerGlobalHotkey(const QString &hotkey);

    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    QGroupBox *apiGroupBox;
    QHBoxLayout *apiLayout;
    QLabel *apiLabel;
    QLineEdit *apiKeyEdit;

    QGroupBox *hotkeyGroupBox;
    QVBoxLayout *hotkeyLayout;
    QLabel *hotkeyLabel;
    HotkeyWidget *hotkeyWidget;

    QGroupBox *recordingGroupBox;
    QVBoxLayout *recordingLayout;
    QLabel *recordingStatusLabel;
    QLabel *recordingInfoLabel;
    QHBoxLayout *bufferSizeLayout;
    QLabel *bufferSizeLabel;
    QSpinBox *bufferSizeSpinBox;

    QGroupBox *transcriptionGroupBox;
    QVBoxLayout *transcriptionLayout;
    QLabel *transcriptionStatusLabel;
    QLabel *transcriptionTextLabel;
    QHBoxLayout *transcriptionButtonLayout;
    QPushButton *startTranscriptionButton;
    QPushButton *stopTranscriptionButton;

    // Global hotkey manager
    GlobalHotkeyManager *m_globalHotkeyManager;

    // Audio recorder
    AudioRecorder *m_audioRecorder;
};

#endif // MAINWINDOW_H