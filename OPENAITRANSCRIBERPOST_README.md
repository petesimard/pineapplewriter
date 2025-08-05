# OpenAITranscriberPost Class

The `OpenAITranscriberPost` class provides a simple interface for uploading audio data to the OpenAI API for transcription using the `/v1/audio/transcriptions` endpoint.

## Features

- Uploads audio data from an `AudioBuffer` to OpenAI's transcription API
- Supports WAV file format (automatically converts PCM data)
- Configurable model selection (default: "gpt-4o-transcribe")
- Thread-safe operation with mutex protection
- Signal-based communication for transcription results and errors

## Usage

### Basic Setup

```cpp
#include "openaitranscriber_post.h"
#include "audiobuffer.h"
#include "fixedbufferdevice.h"

// Create audio buffer
AudioBuffer *audioBuffer = new FixedBufferDevice(1024 * 1024, this);

// Create transcriber
OpenAITranscriberPost *transcriber = new OpenAITranscriberPost(this);
transcriber->setAudioBuffer(audioBuffer);
transcriber->setApiKey("your-openai-api-key");
transcriber->setModel("gpt-4o-transcribe"); // Optional, defaults to "gpt-4o-transcribe"
```

### Connecting Signals

```cpp
// Connect to transcription results
connect(transcriber, &OpenAITranscriberPost::transcriptionReceived,
        this, &YourClass::onTranscriptionReceived);
connect(transcriber, &OpenAITranscriberPost::transcriptionError,
        this, &YourClass::onTranscriptionError);
connect(transcriber, &OpenAITranscriberPost::transcriptionStarted,
        this, &YourClass::onTranscriptionStarted);
connect(transcriber, &OpenAITranscriberPost::transcriptionFinished,
        this, &YourClass::onTranscriptionFinished);
```

### Starting Transcription

```cpp
// Start transcription (reads audio data from buffer)
transcriber->transcribeAudio();

// Check if transcription is in progress
if (transcriber->isTranscribing()) {
    qDebug() << "Transcription in progress...";
}
```

## API Reference

### Public Methods

- `void setApiKey(const QString &apiKey)` - Set the OpenAI API key
- `void setAudioBuffer(AudioBuffer *buffer)` - Set the audio buffer to read from
- `void setModel(const QString &model)` - Set the transcription model (default: "gpt-4o-transcribe")
- `void transcribeAudio()` - Start transcription process
- `bool isTranscribing() const` - Check if transcription is currently in progress

### Signals

- `void transcriptionReceived(const QString &text)` - Emitted when transcription is received
- `void transcriptionError(const QString &error)` - Emitted when an error occurs
- `void transcriptionStarted()` - Emitted when transcription starts
- `void transcriptionFinished()` - Emitted when transcription finishes

## Audio Format

The class automatically converts PCM audio data to WAV format before uploading to the OpenAI API. The WAV file is created with the following specifications:

- Sample Rate: 16,000 Hz
- Channels: 1 (Mono)
- Bits per Sample: 16
- Format: PCM

## Error Handling

The class provides comprehensive error handling:

- API key validation
- Audio buffer validation
- Network error handling
- JSON response parsing
- OpenAI API error message extraction

## Thread Safety

The class uses mutex protection to ensure thread-safe operation when multiple threads might access the transcriber simultaneously.

## Example Integration

See `src/example_usage_post.cpp` for a complete example of how to integrate the `OpenAITranscriberPost` class into your application.

## Differences from OpenAITranscriberRealtime

- **OpenAITranscriberRealtime**: Uses WebSocket connection for real-time streaming transcription
- **OpenAITranscriberPost**: Uses HTTP POST requests for batch transcription of complete audio segments

Choose the appropriate class based on your use case:
- Use `OpenAITranscriberRealtime` for live, real-time transcription
- Use `OpenAITranscriberPost` for transcribing complete audio segments after recording 