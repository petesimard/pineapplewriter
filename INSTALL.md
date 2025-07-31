# Quick Installation Guide

## Prerequisites

Make sure you have the required dependencies installed:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools
```

## Quick Build

1. **Build the application**:
   ```bash
   ./build.sh
   ```

2. **Run the application**:
   ```bash
   cd build && ./bin/SystemTrayApp
   ```

## Features

✅ **System Tray Integration**: Appears in system tray with right-click menu  
✅ **ElevenLabs API Key**: Secure password field for API key storage  
✅ **Custom Hotkey**: Click-to-set hotkey functionality  
✅ **Settings Persistence**: Automatically saves your preferences  
✅ **Linux Mint Compatible**: Works on default Linux Mint installations  

## Usage

1. **Launch**: The app starts and appears in your system tray
2. **Open UI**: Left-click tray icon or right-click → "Open"
3. **Configure API**: Enter your ElevenLabs API key (saved automatically)
4. **Set Hotkey**: Click the hotkey button and press your desired combination
5. **Close Window**: App continues running in system tray

## Troubleshooting

- **Build fails**: Make sure Qt6 development packages are installed
- **Tray not visible**: Some desktop environments may not support system tray
- **Hotkey not working**: The hotkey detection works within the app only

## System Requirements

- Linux Mint (or Ubuntu-based distribution)
- Qt6 development libraries
- CMake 3.16+
- C++17 compatible compiler 