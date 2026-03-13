cmake_minimum_required(VERSION 3.20)

project(YUVPlayer LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure Qt
set(CMAKE_PREFIX_PATH "e:/Qt/6.10.2/mingw_64")
set(QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT 3.20)
find_package(Qt6 COMPONENTS Widgets Multimedia REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(CORE_SOURCES
    src/core/yuv_reader.c
    src/core/yuv_converter.c
    src/core/yuv_converter_sse.c
    src/core/thread_pool.c
    src/core/srt_parser.cpp
)

set(GUI_SOURCES
    src/gui/mainwindow.cpp
    src/gui/videowidget.cpp
    src/gui/mobile_player.h
    src/gui/mobile_player.cpp
    src/main.cpp
    src/resources/app.qrc
    src/resources/app.rc
)

# Include directories
include_directories(src)
include_directories(src/core)
include_directories(src/gui)

# Add Executable for Player
add_executable(YUVPlayer
    ${CORE_SOURCES}
    ${GUI_SOURCES}
)

target_link_libraries(YUVPlayer PRIVATE Qt6::Widgets Qt6::Multimedia)

if(ANDROID)
    set_property(TARGET YUVPlayer PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()

if(ANDROID)
    qt_finalize_executable(YUVPlayer)
endif()

# Add Executable for Benchmark
add_executable(YUVBenchmark
    src/benchmark.cpp
    ${CORE_SOURCES}
)
