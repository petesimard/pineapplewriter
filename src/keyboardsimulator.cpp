#include "keyboardsimulator.h"
#include <QDebug>
#include <QProcess>
#include <QThread>
#include "openaitranscriber_realtime.h"

KeyboardSimulator::KeyboardSimulator()
    : m_available(false)
{
    m_available = checkXdotoolAvailable();
    if (!m_available)
    {
        qWarning() << "xdotool not available - keyboard simulation will be disabled";
    }
}

KeyboardSimulator::~KeyboardSimulator()
{
}

bool KeyboardSimulator::isAvailable() const
{
    return m_available;
}

void KeyboardSimulator::onStreamingStarted()
{
    m_doesNeedSpace = false;
}

bool KeyboardSimulator::checkXdotoolAvailable()
{
    QProcess process;
    process.start("which", QStringList() << "xdotool");
    process.waitForFinished();
    return process.exitCode() == 0;
}

bool KeyboardSimulator::typeText(const QString &text)
{
    if (!m_available)
    {
        qWarning() << "Keyboard simulator not available";
        return false;
    }

    // Use xdotool to type the text
    QProcess process;
    QStringList arguments;

    if (m_doesNeedSpace)
    {
        arguments << "key" << "space";
    }

    arguments << "type" << text;

    process.start("xdotool", arguments);
    bool success = process.waitForFinished(5000); // 5 second timeout

    if (!success)
    {
        qWarning() << "xdotool process timed out";
        return false;
    }

    if (process.exitCode() != 0)
    {
        qWarning() << "xdotool failed with exit code:" << process.exitCode();
        qWarning() << "Error output:" << process.readAllStandardError();
        return false;
    }

    m_doesNeedSpace = text.endsWith(".") || text.endsWith("!") || text.endsWith("?");

    qDebug() << "Successfully typed text using xdotool:" << text;
    return true;
}