#ifndef OPENAITRANSCRIBER_H
#define OPENAITRANSCRIBER_H

#include <QObject>
#include <QWebSocket>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class FixedBufferDevice;

class OpenAITranscriber : public QObject
{
    Q_OBJECT

public:
    explicit OpenAITranscriber(QObject *parent = nullptr);
    ~OpenAITranscriber();

    void setApiKey(const QString &apiKey);
    void setCircularBuffer(FixedBufferDevice *buffer);
    void startStreaming();
    void stopStreaming();
    bool isStreaming() const;

signals:
    void transcriptionReceived(const QString &text);
    void transcriptionError(const QString &error);
    void streamingStarted();
    void streamingStopped();

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);
    void onWebSocketTextMessageReceived(const QString &message);
    void onTimerTimeout();

private:
    QWebSocket *m_webSocket;
    QThread *m_workerThread;
    QTimer *m_timer;
    FixedBufferDevice *m_circularBuffer;
    QString m_apiKey;
    bool m_isStreaming;
    QMutex m_mutex;
    QByteArray m_lastProcessedData;
    QString m_sessionId;
    QString m_currentItemId;

    void setupWebSocket();
    void sendSessionUpdate();
    void sendAudioBuffer(const QByteArray &audioData);
    QByteArray encodeBase64(const QByteArray &data);
    QJsonObject createSessionUpdateMessage();
    void processTranscriptionMessage(const QJsonObject &message);
    void processCommittedMessage(const QJsonObject &message);
};

#endif // OPENAITRANSCRIBER_H