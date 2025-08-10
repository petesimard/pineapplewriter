cmake_minimum_required(VERSION 3.16)
project(PineappleWriter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Multimedia Network WebSockets)

find_package(PkgConfig REQUIRED)
pkg_check_modules(X11 REQUIRED x11 xtst)

# Set Qt6 specific properties
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Fetch QHotkey from GitHub
include(FetchContent)
FetchContent_Declare(
    qhotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey.git
    GIT_TAG master
)
FetchContent_MakeAvailable(qhotkey)


# Create executable
add_executable(PineappleWriter
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/hotkeywidget.cpp
    src/hotkeywidget.h
    src/globalhotkeymanager.cpp
    src/globalhotkeymanager.h
    src/audiorecorder.cpp
    src/audiorecorder.h
    src/audiobuffer.cpp
    src/audiobuffer.h
    src/circularbufferdevice.cpp
    src/circularbufferdevice.h
    src/fixedbufferdevice.cpp
    src/fixedbufferdevice.h
    src/openaitranscriber_realtime.cpp
    src/openaitranscriber_realtime.h
    src/openaitranscriber.cpp
    src/openaitranscriber.h
    src/keyboardsimulator.cpp
    src/keyboardsimulator.h
    src/postprocess.cpp
    src/postprocess.h
    src/pushtotalk.cpp
    src/pushtotalk.h
    resources/resources.qrc
)

# Link Qt6 libraries
target_link_libraries(PineappleWriter
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Network
    Qt6::WebSockets
    ${X11_LIBRARIES}
)
target_link_libraries(PineappleWriter qhotkey)

# Set output directory
set_target_properties(PineappleWriter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS PineappleWriter
    RUNTIME DESTINATION bin
) 


# Install app icon under hicolor theme with the application id as the icon name
install(FILES
    data/icons/io.github.petesimard.pineapplewriter.png
    DESTINATION share/icons/hicolor/256x256/apps
    RENAME io.github.petesimard.pineapplewriter.png
)

# Install desktop integration files and icon for packaging (e.g., Flatpak)
install(FILES
    data/io.github.petesimard.pineapplewriter.desktop
    DESTINATION share/applications
)

install(FILES
    data/io.github.petesimard.pineapplewriter.metainfo.xml
    DESTINATION share/metainfo
)