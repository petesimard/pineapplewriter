#include <QCoreApplication>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioFormat>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Testing volume control functionality...";

    // Get available audio input devices
    const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
    if (inputDevices.isEmpty())
    {
        qWarning() << "No audio input devices found!";
        return 1;
    }

    qDebug() << "Available audio input devices:";
    for (const QAudioDevice &device : inputDevices)
    {
        qDebug() << "  -" << device.description() << "(ID:" << device.id() << ")";
    }

    // Use the first available device
    QAudioDevice device = inputDevices.first();
    qDebug() << "Using device:" << device.description();

    // Set up audio format
    QAudioFormat format;
    format.setSampleRate(24000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!device.isFormatSupported(format))
    {
        qWarning() << "Format not supported, using preferred format";
        format = device.preferredFormat();
    }

    // Create audio source
    QAudioSource *audioSource = new QAudioSource(device, format);

    if (audioSource->isNull())
    {
        qWarning() << "Failed to create audio source!";
        return 1;
    }

    qDebug() << "Audio source created successfully";
    qDebug() << "Initial volume:" << audioSource->volume();

    // Test volume control
    qDebug() << "Testing volume control...";

    // Set volume to 50%
    audioSource->setVolume(0.5);
    qDebug() << "Volume set to 50%:" << audioSource->volume();

    // Set volume to 25%
    audioSource->setVolume(0.25);
    qDebug() << "Volume set to 25%:" << audioSource->volume();

    // Set volume to 75%
    audioSource->setVolume(0.75);
    qDebug() << "Volume set to 75%:" << audioSource->volume();

    // Test volume clamping (should clamp to 1.0)
    audioSource->setVolume(1.5);
    qDebug() << "Volume set to 150% (should clamp to 100%):" << audioSource->volume();

    // Test volume clamping (should clamp to 0.0)
    audioSource->setVolume(-0.5);
    qDebug() << "Volume set to -50% (should clamp to 0%):" << audioSource->volume();

    // Clean up
    delete audioSource;

    qDebug() << "Volume control test completed successfully!";

    return 0;
}