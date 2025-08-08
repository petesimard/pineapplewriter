#include "openaitranscriber_post.h"
#include "audiobuffer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QBuffer>
#include <QRandomGenerator>
#include <QCryptographicHash>

OpenAITranscriberPost::OpenAITranscriberPost(QObject *parent)
    : QObject(parent), m_networkManager(nullptr), m_currentReply(nullptr), m_audioBuffer(nullptr), m_isTranscribing(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_model = "gpt-4o-transcribe";
}

OpenAITranscriberPost::~OpenAITranscriberPost()
{
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void OpenAITranscriberPost::setSystemPrompt(const QString &systemPrompt)
{
    m_systemPrompt = systemPrompt;
}

void OpenAITranscriberPost::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void OpenAITranscriberPost::setAudioBuffer(AudioBuffer *buffer)
{
    m_audioBuffer = buffer;
}

void OpenAITranscriberPost::setModel(const QString &model)
{
    m_model = model;
}

void OpenAITranscriberPost::transcribeAudio()
{
    QMutexLocker locker(&m_mutex);

    if (m_isTranscribing)
    {
        qDebug() << "Already transcribing, ignoring request";
        return;
    }

    if (m_apiKey.isEmpty())
    {
        emit transcriptionError("API key not set");
        return;
    }

    if (!m_audioBuffer)
    {
        emit transcriptionError("Audio buffer not set");
        return;
    }

    // Get audio data from buffer
    QByteArray audioData = m_audioBuffer->readAndClear();
    if (audioData.isEmpty())
    {
        emit transcriptionError("No audio data available");
        return;
    }

    m_isTranscribing = true;
    emit transcriptionStarted();

    // Create multipart form data
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // Add the audio file part
    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/wav"));
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"audio.wav\""));

    // Create WAV file from PCM data
    QByteArray wavData = createAudioFile(audioData);
    audioPart.setBody(wavData);
    multiPart->append(audioPart);

    // Add the model part
    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"model\""));
    modelPart.setBody(m_model.toUtf8());
    multiPart->append(modelPart);

    if (!m_systemPrompt.isEmpty())
    {
        QHttpPart systemPromptPart;
        systemPromptPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"prompt\""));
        systemPromptPart.setBody(m_systemPrompt.toUtf8());
        multiPart->append(systemPromptPart);
    }

    // Create the request
    QNetworkRequest request(QUrl("https://api.openai.com/v1/audio/transcriptions"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    // Send the request
    m_currentReply = m_networkManager->post(request, multiPart);
    multiPart->setParent(m_currentReply); // Delete the multiPart with the reply

    // Connect signals
    connect(m_currentReply, &QNetworkReply::finished, this, &OpenAITranscriberPost::onNetworkReplyFinished);
    connect(m_currentReply, &QNetworkReply::errorOccurred,
            this, &OpenAITranscriberPost::onNetworkReplyError);

    qDebug() << "Sending transcription request to OpenAI API with model:" << m_model << "file size:" << audioData.size();
}

bool OpenAITranscriberPost::isTranscribing() const
{
    return m_isTranscribing;
}

void OpenAITranscriberPost::onNetworkReplyFinished()
{
    QMutexLocker locker(&m_mutex);

    if (!m_currentReply)
    {
        return;
    }

    if (m_currentReply->error() == QNetworkReply::NoError)
    {
        QByteArray responseData = m_currentReply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error == QJsonParseError::NoError)
        {
            QJsonObject jsonObj = doc.object();
            QString text = jsonObj["text"].toString();

            if (!text.isEmpty())
            {
                emit transcriptionReceived(text);
                // qDebug() << "Transcription received:" << text;
            }
            else
            {
                emit transcriptionError("No transcription text in response");
            }
        }
        else
        {
            emit transcriptionError(QString("Failed to parse response: %1").arg(parseError.errorString()));
        }
    }
    else
    {
        QString errorString = m_currentReply->errorString();
        QByteArray errorData = m_currentReply->readAll();

        // Try to parse error response for more details
        QJsonParseError parseError;
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData, &parseError);
        if (parseError.error == QJsonParseError::NoError)
        {
            QJsonObject errorObj = errorDoc.object();
            QString errorMessage = errorObj["error"].toObject()["message"].toString();
            if (!errorMessage.isEmpty())
            {
                errorString = errorMessage;
            }
        }

        emit transcriptionError(QString("Network error: %1").arg(errorString));
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    m_isTranscribing = false;
    emit transcriptionFinished();
}

void OpenAITranscriberPost::onNetworkReplyError(QNetworkReply::NetworkError error)
{
    qWarning() << "Network reply error:" << error;
    // Error handling is done in onNetworkReplyFinished
}

QByteArray OpenAITranscriberPost::createAudioFile(const QByteArray &audioData)
{
    // Create a simple WAV file header for PCM data
    // Assuming 16-bit PCM, 16kHz sample rate, mono channel
    QByteArray wavData;

    // WAV file header (44 bytes)
    const int sampleRate = 16000;
    const int numChannels = 1;
    const int bitsPerSample = 16;
    const int dataSize = audioData.size();
    const int fileSize = 36 + dataSize;

    // Helper function to convert to little-endian bytes
    auto toLittleEndian = [](quint32 value) -> QByteArray
    {
        QByteArray bytes;
        bytes.append(static_cast<char>(value & 0xFF));
        bytes.append(static_cast<char>((value >> 8) & 0xFF));
        bytes.append(static_cast<char>((value >> 16) & 0xFF));
        bytes.append(static_cast<char>((value >> 24) & 0xFF));
        return bytes;
    };

    auto toLittleEndian16 = [](quint16 value) -> QByteArray
    {
        QByteArray bytes;
        bytes.append(static_cast<char>(value & 0xFF));
        bytes.append(static_cast<char>((value >> 8) & 0xFF));
        return bytes;
    };

    // RIFF header
    wavData.append("RIFF", 4);
    wavData.append(toLittleEndian(fileSize));
    wavData.append("WAVE", 4);

    // fmt chunk
    wavData.append("fmt ", 4);
    wavData.append(toLittleEndian(16));  // fmt chunk size
    wavData.append(toLittleEndian16(1)); // PCM format
    wavData.append(toLittleEndian16(numChannels));
    wavData.append(toLittleEndian(sampleRate));
    wavData.append(toLittleEndian(sampleRate * numChannels * bitsPerSample / 8)); // byte rate
    wavData.append(toLittleEndian16(numChannels * bitsPerSample / 8));            // block align
    wavData.append(toLittleEndian16(bitsPerSample));

    // data chunk
    wavData.append("data", 4);
    wavData.append(toLittleEndian(dataSize));
    wavData.append(audioData);

    return wavData;
}