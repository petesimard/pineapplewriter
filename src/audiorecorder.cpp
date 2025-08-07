#include "audiorecorder.h"
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioSource>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QAudioInput>
#include <QFile>
#include <QUrl>
#include <QThread>
#include "fixedbufferdevice.h"

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent), m_audioInput(nullptr), m_audioSource(nullptr), m_audioBuffer(nullptr), m_isRecording(false), m_transcriber(nullptr), m_currentDevice(QMediaDevices::defaultAudioInput())
{
    setupAudioInput();

    // Create OpenAI transcriber
    m_transcriber = new OpenAITranscriberRealtime(this);
    m_transcriber->setAudioBuffer(m_audioBuffer);

    // Connect transcriber signals
    connect(m_transcriber, &OpenAITranscriberRealtime::transcriptionReceived,
            this, &AudioRecorder::transcriptionReceived);
    connect(m_transcriber, &OpenAITranscriberRealtime::transcriptionError,
            this, &AudioRecorder::transcriptionError);
}

AudioRecorder::~AudioRecorder()
{
    if (m_isRecording)
    {
        stopRecording();
    }

    // Clean up audio resources
    if (m_audioSource)
    {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
    }

    if (m_audioInput)
    {
        delete m_audioInput;
        m_audioInput = nullptr;
    }

    if (m_audioBuffer)
    {
        delete m_audioBuffer;
        m_audioBuffer = nullptr;
    }

    if (m_transcriber)
    {
        delete m_transcriber;
        m_transcriber = nullptr;
    }
}

void AudioRecorder::setupAudioInput()
{
    m_audioBuffer = new FixedBufferDevice(5 * 1024 * 1024, this);

    // Set up audio format
    QAudioFormat format;
    format.setSampleRate(24000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    // Use the current device
    if (m_currentDevice.isNull())
    {
        // Get default audio input device
        const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
        if (inputDevices.isEmpty())
        {
            qWarning() << "No audio input devices found";
            return;
        }
        m_currentDevice = inputDevices.first();
    }

    if (!m_currentDevice.isFormatSupported(format))
    {
        qWarning() << "Default format not supported, trying to use nearest";
        format = m_currentDevice.preferredFormat();
    }

    // Create audio input and source
    m_audioInput = new QAudioInput(m_currentDevice, this);
    m_audioSource = new QAudioSource(m_currentDevice, format, this);
}

bool AudioRecorder::isRecording() const
{
    return m_isRecording;
}

void AudioRecorder::startRecording()
{
    if (m_isRecording)
    {
        qDebug() << "Already recording, ignoring start request";
        return;
    }

    if (!m_audioSource)
    {
        qWarning() << "Audio source not initialized";
        return;
    }

    // Clear previous recording
    clearBuffer();

    m_audioSource->start(m_audioBuffer);
    m_isRecording = true;

    qDebug() << "Started audio recording to buffer";
    emit recordingStarted();
}

void AudioRecorder::stopRecording()
{
    if (!m_isRecording)
    {
        qDebug() << "Not recording, ignoring stop request";
        return;
    }

    if (m_audioSource)
    {
        m_audioSource->stop();
        // Give the audio system time to properly stop
        QThread::msleep(100);
    }
    m_isRecording = false;

    qDebug() << "Stopped audio recording";
    emit recordingStopped();
}

QByteArray AudioRecorder::getRecordedAudio() const
{
    return m_recordedAudio;
}

void AudioRecorder::clearBuffer()
{
    m_recordedAudio.clear();
    if (m_audioBuffer)
    {
        m_audioBuffer->clear();
    }
}

void AudioRecorder::setBufferSize(int sizeInBytes)
{
    if (m_audioBuffer)
    {
        m_audioBuffer->setBufferSize(sizeInBytes);
    }
}

int AudioRecorder::getBufferSize() const
{
    if (m_audioBuffer)
    {
        return m_audioBuffer->getBufferSize();
    }
    return 0;
}

AudioBuffer *AudioRecorder::getAudioBuffer() const
{
    return m_audioBuffer;
}

void AudioRecorder::resetAudioState()
{
    if (m_isRecording)
    {
        stopRecording();
    }

    // Reset the circular buffer
    if (m_audioBuffer)
    {
        m_audioBuffer->clear();
    }

    m_recordedAudio.clear();
    qDebug() << "Audio state reset";
}

// OpenAI transcription methods
void AudioRecorder::setOpenAIApiKey(const QString &apiKey)
{
    if (m_transcriber)
    {
        m_transcriber->setApiKey(apiKey);
    }
}

void AudioRecorder::startTranscription()
{
    if (m_transcriber)
    {
        // Start audio recording first
        if (!m_isRecording)
        {
            startRecording();
        }

        // Then start transcription
        m_transcriber->startStreaming();
    }
}

void AudioRecorder::stopTranscription()
{
    if (m_transcriber)
    {
        // Stop transcription first
        m_transcriber->stopStreaming();

        // Then stop audio recording
        if (m_isRecording)
        {
            stopRecording();
        }
    }
}

bool AudioRecorder::isTranscribing() const
{
    return m_transcriber ? m_transcriber->isStreaming() : false;
}

// Volume control methods
void AudioRecorder::setVolume(qreal volume)
{
    if (m_audioSource)
    {
        // Clamp volume to valid range (0.0 to 1.0)
        volume = qBound(0.0, volume, 1.0);
        m_audioSource->setVolume(volume);
        qDebug() << "Audio input volume set to:" << volume;
    }
}

qreal AudioRecorder::getVolume() const
{
    if (m_audioSource)
    {
        return m_audioSource->volume();
    }
    return 1.0; // Default to full volume if no audio source
}

// Device selection methods
void AudioRecorder::setAudioDevice(const QAudioDevice &device)
{
    if (m_isRecording)
    {
        qWarning() << "Cannot change audio device while recording";
        return;
    }

    // Store the new device
    m_currentDevice = device;

    // Clean up existing audio source
    if (m_audioSource)
    {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
    }

    if (m_audioInput)
    {
        delete m_audioInput;
        m_audioInput = nullptr;
    }

    // Set up audio format
    QAudioFormat format;
    format.setSampleRate(24000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    // Check if the device supports our format
    if (!device.isFormatSupported(format))
    {
        qWarning() << "Device format not supported, trying to use the nearest";
        format = device.preferredFormat();
    }

    // Create new audio input and source with the selected device
    m_audioInput = new QAudioInput(device, this);
    m_audioSource = new QAudioSource(device, format, this);

    qDebug() << "Audio device changed to:" << device.description();
}

QAudioDevice AudioRecorder::getCurrentAudioDevice() const
{
    return m_currentDevice;
}

QList<QAudioDevice> AudioRecorder::getAvailableAudioDevices() const
{
    return QMediaDevices::audioInputs();
}
