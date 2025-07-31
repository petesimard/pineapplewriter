#include "hotkeywidget.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>

HotkeyWidget::HotkeyWidget(QWidget *parent)
    : QWidget(parent), isListening(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    button = new QPushButton("Click to set hotkey", this);
    button->setMinimumHeight(40);
    layout->addWidget(button);

    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(5000); // 5 second timeout

    connect(button, &QPushButton::clicked, this, &HotkeyWidget::startListening);
    connect(timeoutTimer, &QTimer::timeout, this, &HotkeyWidget::stopListening);

    updateButtonText();
}

HotkeyWidget::~HotkeyWidget()
{
    if (isListening)
    {
        stopListening();
    }
}

QString HotkeyWidget::getHotkey() const
{
    return keySequenceToString(currentSequence);
}

void HotkeyWidget::setHotkey(const QString &hotkey)
{
    currentSequence = QKeySequence::fromString(hotkey);
    updateButtonText();
}

void HotkeyWidget::startListening()
{
    if (isListening)
        return;

    isListening = true;
    button->setText("Press key combination... (5s timeout)");
    button->setStyleSheet("QPushButton { background-color: #ffcccc; }");

    // Install event filter on the application
    qApp->installEventFilter(this);

    // Start timeout timer
    timeoutTimer->start();
}

void HotkeyWidget::stopListening()
{
    if (!isListening)
        return;

    isListening = false;
    timeoutTimer->stop();

    // Remove event filter
    qApp->removeEventFilter(this);

    updateButtonText();
}

void HotkeyWidget::updateButtonText()
{
    QString text = keySequenceToString(currentSequence);
    if (text.isEmpty())
    {
        button->setText("Click to set hotkey");
    }
    else
    {
        button->setText("Hotkey: " + text);
    }
    button->setStyleSheet("");
}

bool HotkeyWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (!isListening)
        return false;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        processKeyEvent(keyEvent);
        return true; // Consume the event
    }

    return false;
}

void HotkeyWidget::processKeyEvent(QKeyEvent *event)
{
    int key = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Ignore modifier keys alone
    if (key == Qt::Key_Control || key == Qt::Key_Shift ||
        key == Qt::Key_Alt || key == Qt::Key_Meta)
    {
        return;
    }

    // Create key sequence
    QKeySequence newSequence;

    if (modifiers & Qt::ControlModifier)
    {
        newSequence = QKeySequence(key | Qt::CTRL);
    }
    else if (modifiers & Qt::ShiftModifier)
    {
        newSequence = QKeySequence(key | Qt::SHIFT);
    }
    else if (modifiers & Qt::AltModifier)
    {
        newSequence = QKeySequence(key | Qt::ALT);
    }
    else if (modifiers & Qt::MetaModifier)
    {
        newSequence = QKeySequence(key | Qt::META);
    }
    else
    {
        newSequence = QKeySequence(key);
    }

    // Handle combined modifiers
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier)
    {
        newSequence = QKeySequence(key | Qt::CTRL | Qt::SHIFT);
    }
    else if (modifiers & Qt::ControlModifier && modifiers & Qt::AltModifier)
    {
        newSequence = QKeySequence(key | Qt::CTRL | Qt::ALT);
    }
    else if (modifiers & Qt::ShiftModifier && modifiers & Qt::AltModifier)
    {
        newSequence = QKeySequence(key | Qt::SHIFT | Qt::ALT);
    }

    // Set the new sequence
    currentSequence = newSequence;

    // Stop listening and emit signal
    stopListening();
    emit hotkeyChanged(keySequenceToString(currentSequence));
}

QString HotkeyWidget::keySequenceToString(const QKeySequence &sequence) const
{
    if (sequence.isEmpty())
    {
        return "";
    }

    QString result = sequence.toString();

    // Replace Qt's default format with more readable format
    result = result.replace("Ctrl+", "Ctrl+")
                 .replace("Shift+", "Shift+")
                 .replace("Alt+", "Alt+")
                 .replace("Meta+", "Meta+");

    return result;
}