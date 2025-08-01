# Real-time Audio Transcription with OpenAI

This application now includes real-time audio transcription using OpenAI's Realtime API. The transcription runs in a separate thread and processes audio data every second.

## Features

- **Real-time Transcription**: Streams audio to OpenAI's Realtime API for live transcription
- **Separate Thread**: Transcription runs in its own thread to avoid blocking the UI
- **Automatic Audio Processing**: Sends audio buffers to OpenAI every second
- **WebSocket Connection**: Uses WebSocket for real-time communication with OpenAI
- **Error Handling**: Proper error handling and user feedback

## Setup

1. **Get OpenAI API Key**: You need a valid OpenAI API key with access to the Realtime API
2. **Enter API Key**: In the application, enter your OpenAI API key in the "OpenAI API Key" field
3. **Start Recording**: Use the global hotkey to start audio recording
4. **Start Transcription**: Click "Start Transcription" to begin real-time transcription

## How It Works

### Audio Recording
- The application records audio to a circular buffer
- Audio is captured in PCM16 format at 44.1kHz, 2 channels
- The circular buffer maintains the last 5MB of audio data by default

### Transcription Process
1. **WebSocket Connection**: Establishes a WebSocket connection to `wss://api.openai.com/v1/realtime?intent=transcription`
2. **Session Setup**: Sends a session update message with transcription configuration
3. **Audio Streaming**: Every second, new audio data is sent to OpenAI
4. **Real-time Results**: Transcription results are received and displayed in real-time

### Configuration
The transcription uses the following OpenAI configuration:
- **Model**: `gpt-4o-transcribe`
- **Audio Format**: PCM16
- **Turn Detection**: Server-side VAD with 0.5 threshold
- **Noise Reduction**: Near-field processing
- **Language**: Auto-detected

## Usage

1. **Start the Application**: Launch the application
2. **Enter API Key**: Enter your OpenAI API key
3. **Start Recording**: Press your configured global hotkey to start recording
4. **Start Transcription**: Click "Start Transcription" button
5. **View Results**: Transcription text will appear in the "Real-time Transcription" section
6. **Stop**: Click "Stop Transcription" to stop the transcription process

## Technical Details

### Files Added
- `src/openaitranscriber.h` - Header for OpenAI transcription class
- `src/openaitranscriber.cpp` - Implementation of OpenAI transcription
- Updated `src/audiorecorder.h` and `src/audiorecorder.cpp` - Integration with transcription
- Updated `src/mainwindow.h` and `src/mainwindow.cpp` - UI for transcription control

### Dependencies
- Qt6::Network - For WebSocket connections
- Qt6::Multimedia - For audio recording
- Qt6::Core - For threading and timers

### Threading Model
- The transcription runs in the main thread but uses a timer for periodic processing
- WebSocket communication is asynchronous
- Audio processing happens every second to avoid overwhelming the API

### Error Handling
- Network connection errors are reported to the user
- API key validation is performed before starting transcription
- WebSocket disconnections are handled gracefully

## Troubleshooting

### Common Issues
1. **"API Key Required"**: Make sure you've entered a valid OpenAI API key
2. **"WebSocket error"**: Check your internet connection and API key validity
3. **"Transcription error"**: The API may be rate-limited or the audio format may not be supported

### Debug Information
The application logs debug information to help troubleshoot issues:
- WebSocket connection status
- Audio data processing
- Transcription results
- Error messages

## API Requirements

- OpenAI API key with access to the Realtime API
- Internet connection for WebSocket communication
- Sufficient API credits for transcription requests

## Performance Notes

- Audio is processed every second to balance responsiveness with API usage
- The circular buffer prevents memory issues with long recordings
- WebSocket connection is maintained throughout the transcription session 