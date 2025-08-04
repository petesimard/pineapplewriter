#ifndef GLOBALHOTKEYMANAGER_H
#define GLOBALHOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>
#include <QTimer>
#include <QHotkey>

class PushToTalk;

class GlobalHotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit GlobalHotkeyManager(QObject *parent = nullptr);
    ~GlobalHotkeyManager();

    bool registerHotkey(const QString &hotkeyString);
    bool unregisterHotkey();
    bool isRegistered() const;
    QString getCurrentHotkey() const;

    enum InputMethod
    {
        Toggle,
        PTT
    };

    InputMethod inputMethod = InputMethod::Toggle;

signals:
    void
    hotkeyPressed();
    void pttStateChanged(bool isActive);

private slots:
    void onHotkeyPressed();
    void checkPttState();

private:
    QHotkey *m_hotkey;
    QString m_currentHotkey;
    bool m_isRegistered;
    PushToTalk *m_pushToTalk;
    bool m_isPttActive = false;
    QTimer *m_pttStateTimer;

    bool parseHotkeyString(const QString &hotkeyString, QKeySequence &sequence);
};

#endif // GLOBALHOTKEYMANAGER_H