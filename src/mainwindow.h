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
#include "hotkeywidget.h"
#include "globalhotkeymanager.h"
#include "audiorecorder.h"
#include "keyboardsimulator.h"

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
    void onTranscriptionReceived(const QString &text);
    void onTranscriptionError(const QString &error);
    void startTranscription();
    void stopTranscription();
    void onPttStateChanged(bool isActive);

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

    // Global hotkey manager
    GlobalHotkeyManager *m_globalHotkeyManager;

    // Audio recorder
    AudioRecorder *m_audioRecorder;

    // Keyboard simulator
    KeyboardSimulator *m_keyboardSimulator;

    // System tray
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_openAction;
    QAction *m_quitAction;
    QPixmap m_defaultIcon;
    QPixmap m_recordingIcon;

    void setupSystemTray();
    void updateTrayIcon(bool isRecording);
    QPixmap createRecordingIcon();
};

#endif // MAINWINDOW_H