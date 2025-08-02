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
#include "openaitranscriber.h"

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent), m_audioInput(nullptr), m_audioSource(nullptr), m_audioBuffer(nullptr), m_isRecording(false), m_transcriber(nullptr)
{
    setupAudioInput();

    // Create OpenAI transcriber
    m_transcriber = new OpenAITranscriber(this);
    m_transcriber->setAudioBuffer(m_audioBuffer);

    // Connect transcriber signals
    connect(m_transcriber, &OpenAITranscriber::transcriptionReceived,
            this, &AudioRecorder::transcriptionReceived);
    connect(m_transcriber, &OpenAITranscriber::transcriptionError,
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

    // Get default audio input device
    const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
    if (inputDevices.isEmpty())
    {
        qWarning() << "No audio input devices found";
        return;
    }

    QAudioDevice inputDevice = inputDevices.first();
    if (!inputDevice.isFormatSupported(format))
    {
        qWarning() << "Default format not supported, trying to use nearest";
        format = inputDevice.preferredFormat();
    }

    // Create audio input and source
    m_audioInput = new QAudioInput(inputDevice, this);
    m_audioSource = new QAudioSource(inputDevice, format, this);
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

    // Start recording to circular buffer
    m_audioSource->start(m_audioBuffer);
    m_isRecording = true;

    qDebug() << "Started audio recording to circular buffer";
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

    // Capture the recorded audio
    captureSystemAudio();

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

void AudioRecorder::captureSystemAudio()
{
    // Get the recorded data from circular buffer
    if (m_audioBuffer)
    {
        m_recordedAudio = m_audioBuffer->getData();
        qDebug() << "Captured" << m_recordedAudio.size() << "bytes of audio data from circular buffer";

        // Save captured audio to file
        QFile file("test.pcm");
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(m_recordedAudio);
            file.close();
            qDebug() << "Saved audio data to test.pcm";
        }
        else
        {
            qWarning() << "Failed to save audio data to test.pcm:" << file.errorString();
        }
    }
    else
    {
        qWarning() << "Circular buffer not initialized";
    }
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
