#include "globalhotkeymanager.h"
#include <QDebug>
#include <QApplication>

GlobalHotkeyManager::GlobalHotkeyManager(QObject *parent)
    : QObject(parent)
#ifdef USE_QHOTKEY
      ,
      m_hotkey(nullptr)
#else
      ,
      m_hotkey(nullptr)
#endif
      ,
      m_isRegistered(false)
{
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
    unregisterHotkey();
}

bool GlobalHotkeyManager::registerHotkey(const QString &hotkeyString)
{
    // Unregister any existing hotkey first
    unregisterHotkey();

    if (hotkeyString.isEmpty())
    {
        return false;
    }

#ifdef USE_QHOTKEY
    // Parse the hotkey string
    QKeySequence sequence;
    if (!parseHotkeyString(hotkeyString, sequence))
    {
        qWarning() << "Failed to parse hotkey string:" << hotkeyString;
        return false;
    }

    // Create new QHotkey instance
    m_hotkey = new QHotkey(sequence, true, this);

    // Connect the signal
    connect(m_hotkey, &QHotkey::activated, this, &GlobalHotkeyManager::onHotkeyPressed);

    // Try to register the hotkey
    if (m_hotkey->isRegistered())
    {
        m_currentHotkey = hotkeyString;
        m_isRegistered = true;
        // qDebug() << "Successfully registered global hotkey:" << hotkeyString;
        return true;
    }
    else
    {
        qWarning() << "Failed to register global hotkey:" << hotkeyString;
        delete m_hotkey;
        m_hotkey = nullptr;
        return false;
    }
#else
    // Fallback implementation - just store the hotkey string
    m_currentHotkey = hotkeyString;
    m_isRegistered = true;
    qDebug() << "QHotkey not available - hotkey stored but not registered globally:" << hotkeyString;
    qDebug() << "To enable global hotkeys, install QHotkey library";
    return true;
#endif
}

bool GlobalHotkeyManager::unregisterHotkey()
{
#ifdef USE_QHOTKEY
    if (m_hotkey)
    {
        delete m_hotkey;
        m_hotkey = nullptr;
    }
#endif

    m_currentHotkey.clear();
    m_isRegistered = false;
    return true;
}

bool GlobalHotkeyManager::isRegistered() const
{
#ifdef USE_QHOTKEY
    return m_isRegistered && m_hotkey && m_hotkey->isRegistered();
#else
    return m_isRegistered;
#endif
}

QString GlobalHotkeyManager::getCurrentHotkey() const
{
    return m_currentHotkey;
}

void GlobalHotkeyManager::onHotkeyPressed()
{
    emit hotkeyPressed();
}

bool GlobalHotkeyManager::parseHotkeyString(const QString &hotkeyString, QKeySequence &sequence)
{
    // Handle common hotkey formats
    QString normalized = hotkeyString;

    // Replace common modifiers with Qt format
    normalized = normalized.replace("Ctrl+", "Ctrl+")
                     .replace("Shift+", "Shift+")
                     .replace("Alt+", "Alt+")
                     .replace("Meta+", "Meta+");

    // Try to parse as QKeySequence
    sequence = QKeySequence::fromString(normalized);

    if (sequence.isEmpty())
    {
        // Try alternative parsing for custom formats
        QStringList parts = hotkeyString.split("+", Qt::SkipEmptyParts);
        if (parts.isEmpty())
        {
            return false;
        }

        int key = 0;
        for (const QString &part : parts)
        {
            QString trimmed = part.trimmed();
            if (trimmed.toLower() == "ctrl")
            {
                key |= Qt::CTRL;
            }
            else if (trimmed.toLower() == "shift")
            {
                key |= Qt::SHIFT;
            }
            else if (trimmed.toLower() == "alt")
            {
                key |= Qt::ALT;
            }
            else if (trimmed.toLower() == "meta")
            {
                key |= Qt::META;
            }
            else
            {
                // Try to parse as a key
                QKeySequence keySeq(trimmed);
                if (!keySeq.isEmpty())
                {
                    key |= static_cast<int>(keySeq[0]) & ~(Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::META);
                }
            }
        }

        if (key != 0)
        {
            sequence = QKeySequence(key);
        }
    }

    return !sequence.isEmpty();
}