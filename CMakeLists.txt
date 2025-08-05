cmake_minimum_required(VERSION 3.16)
project(SystemTrayApp VERSION 1.0.0 LANGUAGES CXX)

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

# Try to find QHotkey, but make it optional
find_package(QHotkey REQUIRED)


# Create executable
add_executable(SystemTrayApp
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
    src/openaitranscriber_post.cpp
    src/openaitranscriber_post.h
    src/keyboardsimulator.cpp
    src/keyboardsimulator.h
    src/postprocess.cpp
    src/postprocess.h
    src/pushtotalk.cpp
    src/pushtotalk.h
    resources/resources.qrc
)

# Link Qt6 libraries
target_link_libraries(SystemTrayApp
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Network
    Qt6::WebSockets
    ${X11_LIBRARIES}
)
target_link_libraries(SystemTrayApp qhotkey)

# Set output directory
set_target_properties(SystemTrayApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS SystemTrayApp
    RUNTIME DESTINATION bin
) 