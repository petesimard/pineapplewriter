# Pineapple Writer

![Logo](https://github.com/petesimard/pineapplewriter/blob/main/data/icons/io.github.petesimard.pineapplewriter.png)


A Qt6-based system tray application that enables real time Speech to Text (STT) using OpenAI. Tested on Linux Mint but should work on most Linux distros. No wayland support.

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


## Building the Application

A helper builder shell script is included to install the requirements and make & install

```
./build.sh
```

## Usage

1. **Launch the application**:
   - ./build/bin/PineappleWriter
   - The app will start and appear in your system tray
2. **Access the UI**: 
   - Left-click the tray icon to open the main window
   - Right-click for menu options (Open/Quit)
3. **Start Recording**:
   - The default hotkey is right alt. Hold right alt down while speaking and release to finish.
4. **Recording Status**: 
   - A green circle appears on the system tray icon when recording is active
   - A yellow circle appears when a transcription is being processed
5. **Configure OpenAI API**:
   - Enter your OpenAI API key in the text field
   - The key is automatically saved and hidden for security
6. **Close the window**: The app continues running in the system tray. To close right click on the system tray icon and select close.

## Troubleshooting

### Common Issues

1. **Qt6 not found**: Ensure you have Qt6 development packages installed
2. **System tray not available**: Some desktop environments may not support system tray

### Dependencies

- **Qt6**: Core, Widgets, and Gui modules
- **CMake**: Version 3.16 or higher
- **C++17**: Compiler with C++17 support

## License

MIT License

Copyright (c) 2025 Peter Simard

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN