# System Tray Application

A Qt6-based system tray application that provides a user interface for configuring ElevenLabs API keys and custom hotkeys.

## Features

- **System Tray Integration**: Runs in the system tray with right-click menu (Open/Quit)
- **Recording Status Indicator**: Green circle overlay on system tray icon when recording is active
- **ElevenLabs API Configuration**: Secure text input field for storing API keys
- **Custom Hotkey Support**: Click-to-set hotkey functionality with visual feedback
- **Settings Persistence**: Automatically saves and loads user preferences
- **Linux Mint Compatible**: Designed to work on default Linux Mint installations

## Prerequisites

### For Linux Mint

Install the required dependencies:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools
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
   ./bin/SystemTrayApp
   ```

## Installation

To install the application system-wide:

```bash
# From the build directory
sudo make install
```

The application will be installed to `/usr/local/bin/SystemTrayApp`.

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
SystemTrayApp/
├── CMakeLists.txt          # Main build configuration
├── README.md              # This file
├── src/
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.h       # Main window header
│   ├── mainwindow.cpp     # Main window implementation
│   ├── hotkeywidget.h     # Hotkey widget header
│   └── hotkeywidget.cpp   # Hotkey widget implementation
└── resources/
    └── resources.qrc      # Qt resource file
```

## Development

### Key Components

- **MainWindow**: Main application window with UI elements
- **HotkeyWidget**: Custom widget for capturing key combinations
- **System Tray**: Handles tray icon and context menu
- **Settings**: Automatic persistence using QSettings

### Building for Development

For development with debugging:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### VS Code Debugging

The project includes VS Code debug configuration:

1. **Press F5** to build and debug the application
2. **Ctrl+Shift+P** → "Tasks: Run Task" → "build-debug" for manual debug build
3. **Ctrl+Shift+P** → "Tasks: Run Task" → "build-release" for release build
4. **Ctrl+Shift+P** → "Tasks: Run Task" → "clean" to clean all build directories

The debug configuration will:
- Build the project in debug mode with symbols
- Launch the debugger (GDB)
- Allow setting breakpoints and stepping through code
- Show variable values and call stack

## Troubleshooting

### Common Issues

1. **Qt6 not found**: Ensure you have Qt6 development packages installed
2. **System tray not available**: Some desktop environments may not support system tray
3. **Hotkey not working**: The hotkey detection works within the application only

### Dependencies

- **Qt6**: Core, Widgets, and Gui modules
- **CMake**: Version 3.16 or higher
- **C++17**: Compiler with C++17 support

## License

This project is provided as-is for educational and development purposes. 