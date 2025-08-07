#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QAudioInput>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QByteArray>
#include <QTimer>
#include <QIODevice>
#include "audiobuffer.h"
#include "openaitranscriber_realtime.h"

class AudioRecorder : public QObject
{
    Q_OBJECT

public:
    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder();

    bool isRecording() const;
    void startRecording();
    void stopRecording();
    QByteArray getRecordedAudio() const;
    void clearBuffer();
    void setBufferSize(int sizeInBytes);
    int getBufferSize() const;
    void resetAudioState();

    // OpenAI transcription methods
    void setOpenAIApiKey(const QString &apiKey);
    void startTranscription();
    void stopTranscription();
    bool isTranscribing() const;
    AudioBuffer *getAudioBuffer() const;

    // Volume control methods
    void setVolume(qreal volume);
    qreal getVolume() const;

    // Device selection methods
    void setAudioDevice(const QAudioDevice &device);
    QAudioDevice getCurrentAudioDevice() const;
    QList<QAudioDevice> getAvailableAudioDevices() const;

signals:
    void recordingStarted();
    void recordingStopped();
    void recordingError(const QString &error);
    void transcriptionReceived(const QString &text);
    void transcriptionError(const QString &error);

private:
    QAudioInput *m_audioInput;
    QAudioSource *m_audioSource;
    AudioBuffer *m_audioBuffer;
    QByteArray m_recordedAudio;
    bool m_isRecording;
    OpenAITranscriberRealtime *m_transcriber;
    QAudioDevice m_currentDevice;

    void setupAudioInput();
};

#endif // AUDIORECORDER_H