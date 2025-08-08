# Pineapple Writer

A Qt6-based system tray application that enables real time Speech to Text (STT) using OpenAI.

## Features

- **System Tray Integration**: Runs in the system tray with right-click menu (Open/Quit)
- **OpenAI Integration**: Provide your own API key to enable transcription
- **Custom Hotkey Support**: Click-to-set hotkey functionality with visual feedback
- **Multiple Input Methods**: Use push to talk or enable a hotkey to toggle on/off
- **Customizable**: Change models, hotkeys, prompts, input methods
- **Settings Persistence**: Automatically saves and loads user preferences

## Usage

Right click the pineapple icon in the systemm tray and select Open
Enter your OpenAI API Key
The default input method is Push-To-Talk bound to the right alt key


## Prerequisites

Install the required dependencies:

```bash
apt-get install xdotool
```

## Building the Application

1. **Clone or navigate to the project directory**:
   ```bash
   cd /path/to/your/project
   ```

2. **Create a build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

4. **Build the application**:
   ```bash
   make -j$(nproc)
   ```

5. **Run the application**:
   ```bash
   ./bin/PineappleWriter
   ```

## Installation

To install the application system-wide:

```bash
# From the build directory
sudo make install
```

The application will be installed to `/usr/local/bin/PineappleWriter`.

## Usage

1. **Launch the application**: The app will start and appear in your system tray
2. **Access the UI**: 
   - Left-click the tray icon to open the main window
   - Right-click for menu options (Open/Quit)
3. **Recording Status**: 
   - A green circle appears on the system tray icon when recording is active
   - The tooltip changes to indicate recording status
4. **Configure ElevenLabs API**:
   - Enter your ElevenLabs API key in the text field
   - The key is automatically saved and hidden for security
5. **Set Custom Hotkey**:
   - Click the "Click to set hotkey" button
   - Press your desired key combination (e.g., Ctrl+Shift+A)
   - The hotkey will be captured and displayed
6. **Close the window**: The app continues running in the system tray

## Project Structure

```
PineappleWriter/
├── CMakeLists.txt          # Main build configuration
├── README.md              # This file
├── src/
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.h       # Main window header
│   ├── mainwindow.cpp     # Main window implementation
└── resources/
    └── resources.qrc      # Qt resource file
```

## Development

### Key Components

- **MainWindow**: Main application window with UI elements
- **System Tray**: Handles tray icon and context menu
- **Settings**: Automatic persistence using QSettings

## Troubleshooting

### Common Issues

1. **Qt6 not found**: Ensure you have Qt6 development packages installed
2. **System tray not available**: Some desktop environments may not support system tray

### Dependencies

- **Qt6**: Core, Widgets, and Gui modules
- **CMake**: Version 3.16 or higher
- **C++17**: Compiler with C++17 support

## License
This project is licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

