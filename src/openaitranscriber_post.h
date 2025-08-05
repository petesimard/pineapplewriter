#ifndef OPENAITRANSCRIBERPOST_H
#define OPENAITRANSCRIBERPOST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMutex>

class AudioBuffer;

class OpenAITranscriberPost : public QObject
{
    Q_OBJECT

public:
    explicit OpenAITranscriberPost(QObject *parent = nullptr);
    ~OpenAITranscriberPost();

    void setApiKey(const QString &apiKey);
    void setAudioBuffer(AudioBuffer *buffer);
    void setModel(const QString &model = "gpt-4o-transcribe");
    void transcribeAudio();
    bool isTranscribing() const;

signals:
    void transcriptionReceived(const QString &text);
    void transcriptionError(const QString &error);
    void transcriptionStarted();
    void transcriptionFinished();

private slots:
    void onNetworkReplyFinished();
    void onNetworkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    AudioBuffer *m_audioBuffer;
    QString m_apiKey;
    QString m_model;
    bool m_isTranscribing;
    QMutex m_mutex;

    QByteArray createMultipartData(const QByteArray &audioData);
    QString generateBoundary();
    QByteArray createAudioFile(const QByteArray &audioData);
};

#endif // OPENAITRANSCRIBERPOST_H