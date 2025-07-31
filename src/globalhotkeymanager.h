#ifndef GLOBALHOTKEYMANAGER_H
#define GLOBALHOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>

#ifdef USE_QHOTKEY
#include <QHotkey>
#endif

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

signals:
    void hotkeyPressed();

private slots:
    void onHotkeyPressed();

private:
#ifdef USE_QHOTKEY
    QHotkey *m_hotkey;
#else
    void *m_hotkey; // Dummy pointer when QHotkey is not available
#endif
    QString m_currentHotkey;
    bool m_isRegistered;

    bool parseHotkeyString(const QString &hotkeyString, QKeySequence &sequence);
};

#endif // GLOBALHOTKEYMANAGER_H