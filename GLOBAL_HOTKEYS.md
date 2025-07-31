# Global Hotkey Registration

This application now supports registering hotkeys with the desktop environment so they work globally (even when the app is in the background).

## How it Works

The application uses the **QHotkey** library to register global hotkeys with the desktop environment:

- **Linux (X11)**: Uses X11 XGrabKey API
- **Linux (Wayland)**: Uses D-Bus portal interface
- **Windows**: Uses Windows API RegisterHotKey
- **macOS**: Uses Carbon RegisterEventHotKey

## Installation

### Option 1: Using vcpkg (Recommended)

```bash
# Install vcpkg if you don't have it
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Install QHotkey
./vcpkg install qhotkey

# Configure CMake with vcpkg
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Option 2: Manual Installation

```bash
# Clone QHotkey repository
git clone https://github.com/Skycoder42/QHotkey.git
cd QHotkey

# Build and install
mkdir build && cd build
cmake ..
make
sudo make install
```

### Option 3: Using Conan

```bash
# Install Conan if you don't have it
pip install conan

# Add QHotkey to your conanfile.txt
qhotkey/1.4.1@skycoder42/stable

# Install dependencies
conan install . --build=missing
```

## Building the Application

After installing QHotkey, build the application:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

1. **Set a Hotkey**: Click the "Click to set hotkey" button and press your desired key combination (e.g., Ctrl+Shift+A)

2. **Global Registration**: The application will automatically register the hotkey with the desktop environment

3. **Test the Hotkey**: Press the registered hotkey combination from anywhere on your system - it should bring up the application window

## Troubleshooting

### Hotkey Registration Fails

If you see "Failed to register global hotkey" messages:

1. **Check if QHotkey is installed**: The application will show a message if QHotkey is not found
2. **Check for conflicts**: Another application might be using the same hotkey
3. **Try different combinations**: Some key combinations might be reserved by the system

### Linux-Specific Issues

**X11**: Make sure you have the required X11 development libraries:
```bash
# Ubuntu/Debian
sudo apt-get install libx11-dev libxcb1-dev

# Fedora/RHEL
sudo dnf install libX11-devel libxcb-devel
```

**Wayland**: Ensure you have the required D-Bus portal:
```bash
# Ubuntu/Debian
sudo apt-get install xdg-desktop-portal

# Fedora/RHEL
sudo dnf install xdg-desktop-portal
```

### Windows-Specific Issues

- Ensure you're running with appropriate permissions
- Some antivirus software might block global hotkey registration

### macOS-Specific Issues

- Ensure the application has accessibility permissions
- Go to System Preferences > Security & Privacy > Privacy > Accessibility and add your application

## Fallback Mode

If QHotkey is not available, the application will still work but hotkeys will only function when the application window is focused. You'll see a message in the console indicating that global hotkeys are disabled.

## Customization

You can modify the `onGlobalHotkeyPressed()` function in `mainwindow.cpp` to perform different actions when the hotkey is pressed:

```cpp
void MainWindow::onGlobalHotkeyPressed()
{
    // Show the window
    show();
    raise();
    activateWindow();
    
    // Add your custom functionality here
    // For example:
    // - Start recording audio
    // - Toggle application visibility
    // - Trigger a specific action
}
```

## Security Considerations

- Global hotkeys work system-wide, so choose combinations that won't conflict with other applications
- Be aware that global hotkeys can be used to trigger actions even when the application is not visible
- Consider user privacy and security when implementing hotkey-triggered actions 