#include "openaitranscriber.h"
#include "fixedbufferdevice.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTimer>

OpenAITranscriber::OpenAITranscriber(QObject *parent)
    : QObject(parent), m_webSocket(nullptr), m_workerThread(nullptr), m_timer(nullptr), m_circularBuffer(nullptr), m_isStreaming(false), m_sessionId(""), m_currentItemId("")
{
    // Create timer for periodic audio processing
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &OpenAITranscriber::onTimerTimeout);
}

OpenAITranscriber::~OpenAITranscriber()
{
    stopStreaming();
}

void OpenAITranscriber::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void OpenAITranscriber::setCircularBuffer(FixedBufferDevice *buffer)
{
    m_circularBuffer = buffer;
}

void OpenAITranscriber::startStreaming()
{
    if (m_isStreaming)
    {
        qDebug() << "Already streaming, ignoring start request";
        return;
    }

    if (m_apiKey.isEmpty())
    {
        emit transcriptionError("API key not set");
        return;
    }

    if (!m_circularBuffer)
    {
        emit transcriptionError("Circular buffer not set");
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_isStreaming = true;
    m_lastProcessedData.clear();
    m_sessionId = "";
    m_currentItemId = "";

    setupWebSocket();
    emit streamingStarted();
}

void OpenAITranscriber::stopStreaming()
{
    if (!m_isStreaming)
    {
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_isStreaming = false;

    if (m_timer)
    {
        m_timer->stop();
    }

    if (m_webSocket)
    {
        m_webSocket->close();
        delete m_webSocket;
        m_webSocket = nullptr;
    }

    emit streamingStopped();
}

bool OpenAITranscriber::isStreaming() const
{
    return m_isStreaming;
}

void OpenAITranscriber::setupWebSocket()
{
    if (m_webSocket)
    {
        delete m_webSocket;
    }

    m_webSocket = new QWebSocket();

    // Connect signals
    connect(m_webSocket, &QWebSocket::connected, this, &OpenAITranscriber::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &OpenAITranscriber::onWebSocketDisconnected);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &OpenAITranscriber::onWebSocketError);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &OpenAITranscriber::onWebSocketTextMessageReceived);

    // Connect to OpenAI Realtime API with authentication
    QUrl url("wss://api.openai.com/v1/realtime?intent=transcription");

    // Set up headers for authentication
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    request.setRawHeader("openai-beta", "realtime=v1");
    request.setRawHeader("input_audio_format", "pcm16");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_webSocket->open(request);
}

void OpenAITranscriber::onWebSocketConnected()
{
    qDebug() << "WebSocket connected to OpenAI Realtime API";

    // Send session update message
    // Start timer for periodic audio processing
    m_timer->start();
}

void OpenAITranscriber::onWebSocketDisconnected()
{
    qDebug() << "WebSocket disconnected from OpenAI Realtime API";
    m_timer->stop();

    if (m_isStreaming)
    {
        emit transcriptionError("WebSocket connection lost");
    }
}

void OpenAITranscriber::onWebSocketError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << error;
    emit transcriptionError(QString("WebSocket error: %1").arg(error));
}

void OpenAITranscriber::onWebSocketTextMessageReceived(const QString &message)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failed to parse WebSocket message:" << parseError.errorString();
        return;
    }

    QJsonObject jsonObj = doc.object();
    QString type = jsonObj["type"].toString();

    // qDebug() << "Received message:" << message;

    if (type == "transcription_session.created")
    {
        m_sessionId = jsonObj["session"].toObject()["id"].toString();
        qDebug() << "Transcription session created with ID:" << m_sessionId;

        sendSessionUpdate();
    }
    else if (type == "input_audio_buffer.committed")
    {
        processCommittedMessage(jsonObj);
    }
    else if (type == "conversation.item.input_audio_transcription.delta")
    {
        processTranscriptionMessage(jsonObj);
    }
    else if (type == "error")
    {
        QString errorMessage = jsonObj["error"].toObject()["message"].toString();
        qWarning() << "Received error from OpenAI API:" << errorMessage;
        emit transcriptionError(errorMessage);
    }
    else
    {
        qDebug() << "Received message type:" << type;
    }
}

void OpenAITranscriber::onTimerTimeout()
{
    if (!m_isStreaming || !m_circularBuffer || !m_webSocket)
    {
        return;
    }

    // Get current audio data from circular buffer
    QByteArray currentData = m_circularBuffer->readAndClear();

    if (currentData.isEmpty())
    {
        qDebug() << "No data to send";
        return;
    }

    // Only send new data since last processing
    sendAudioBuffer(currentData);
}

void OpenAITranscriber::sendSessionUpdate()
{
    QJsonObject sessionUpdate = createSessionUpdateMessage();
    QJsonDocument doc(sessionUpdate);
    QString message = doc.toJson(QJsonDocument::Compact);

    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState)
    {
        m_webSocket->sendTextMessage(message);
        qDebug() << "Sent session update message";
    }
}

void OpenAITranscriber::sendAudioBuffer(const QByteArray &audioData)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState || m_sessionId.isEmpty())
    {
        return;
    }

    QJsonObject audioMessage;
    audioMessage["type"] = "input_audio_buffer.append";
    audioMessage["audio"] = QString::fromUtf8(encodeBase64(audioData));

    QJsonDocument doc(audioMessage);
    QString message = doc.toJson(QJsonDocument::Compact);

    qDebug() << "Sending audio buffer message length: " << message.length();

    m_webSocket->sendTextMessage(message);
}

QByteArray OpenAITranscriber::encodeBase64(const QByteArray &data)
{
    return data.toBase64();
}

QJsonObject OpenAITranscriber::createSessionUpdateMessage()
{
    QJsonObject sessionUpdate;
    sessionUpdate["type"] = "transcription_session.update";

    QJsonObject session;

    QJsonObject turnDetection;
    turnDetection["type"] = "server_vad";
    turnDetection["threshold"] = 0.5;
    turnDetection["prefix_padding_ms"] = 300;
    turnDetection["silence_duration_ms"] = 60;
    session["turn_detection"] = turnDetection;

    // QJsonObject noiseReduction;
    // noiseReduction["type"] = "near_field";
    // session["input_audio_noise_reduction"] = noiseReduction;

    QJsonObject inputAudioTranscription;
    inputAudioTranscription["model"] = "gpt-4o-transcribe";

    session["input_audio_transcription"] = inputAudioTranscription;
    sessionUpdate["session"] = session;
    return sessionUpdate;
}

void OpenAITranscriber::processTranscriptionMessage(const QJsonObject &message)
{
    QString text = message["delta"].toString();

    if (!text.isEmpty())
    {
        emit transcriptionReceived(text);
        // qDebug() << "Transcription received:" << text;
    }
}

void OpenAITranscriber::processCommittedMessage(const QJsonObject &message)
{
    QJsonObject committed = message["committed"].toObject();
    m_currentItemId = committed["item_id"].toString();
    QString previousItemId = committed["previous_item_id"].toString();

    qDebug() << "Audio buffer committed - Item ID:" << m_currentItemId << "Previous:" << previousItemId;
}