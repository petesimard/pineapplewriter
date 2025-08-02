#ifndef KEYBOARDSIMULATOR_H
#define KEYBOARDSIMULATOR_H

#include <QString>
#include <QProcess>
#include <QObject>

class KeyboardSimulator : public QObject
{
    Q_OBJECT

public:
    KeyboardSimulator();
    ~KeyboardSimulator();

    bool typeText(const QString &text);
    bool isAvailable() const;
    void onStreamingStarted();

private:
    bool m_available;
    bool m_doesNeedSpace;
    bool checkXdotoolAvailable();
};

#endif // KEYBOARDSIMULATOR_H