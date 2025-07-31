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
#include "hotkeywidget.h"
#include "globalhotkeymanager.h"

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
};

#endif // MAINWINDOW_H