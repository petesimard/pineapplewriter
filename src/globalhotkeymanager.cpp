#include "globalhotkeymanager.h"
#include <QDebug>
#include <QApplication>
#include "pushtotalk.h"

// GlobalHotkeyManager implementation
GlobalHotkeyManager::GlobalHotkeyManager(QObject *parent)
    : QObject(parent),
      m_hotkey(nullptr),
      m_isRegistered(false)
{
    m_pushToTalk = new PushToTalk();
    m_pushToTalk->setCodeCode(Alt_L);

    // Initialize timer to check PushToTalk state
    m_pttStateTimer = new QTimer(this);
    connect(m_pttStateTimer, &QTimer::timeout, this, &GlobalHotkeyManager::checkPttState);
    m_pttStateTimer->start(100); // Check every 100ms
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
}

bool GlobalHotkeyManager::unregisterHotkey()
{
    if (m_hotkey)
    {
        delete m_hotkey;
        m_hotkey = nullptr;
    }

    m_currentHotkey.clear();
    m_isRegistered = false;
    return true;
}

bool GlobalHotkeyManager::isRegistered() const
{
    return m_isRegistered && m_hotkey && m_hotkey->isRegistered();
}

QString GlobalHotkeyManager::getCurrentHotkey() const
{
    return m_currentHotkey;
}

void GlobalHotkeyManager::onHotkeyPressed()
{
    if (inputMethod == InputMethod::Toggle)
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

void GlobalHotkeyManager::checkPttState()
{
    if (m_pushToTalk)
    {
        bool currentPttState = m_pushToTalk->m_isActive;
        if (currentPttState != m_isPttActive)
        {
            m_isPttActive = currentPttState;
            emit pttStateChanged(m_isPttActive);
        }
    }
}