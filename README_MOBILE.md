# Mobile Audio Player with SRT Subtitles

This module provides a mobile-friendly interface for playing `.wav` audio files with synchronized `.srt` subtitles.

## Features
- **Audio Playback**: Supports `.wav` and other formats supported by Qt Multimedia.
- **Subtitle Display**: Parses and displays `.srt` subtitles synchronized with audio.
- **Touch-friendly UI**: Large buttons and simple layout suitable for mobile devices.

## Building for Android
To build this application for Android:
1. Ensure you have **Qt for Android** installed (via Qt Maintenance Tool).
2. Install **Android Studio**, **SDK**, and **NDK**.
3. Configure your Android Kit in Qt Creator.
4. Open `CMakeLists.txt` in Qt Creator.
5. Select the Android Kit.
6. Build and Deploy.

## Usage
1. Launch the application.
2. Click **Open .wav** to select an audio file.
3. Click **Open .srt** to select a subtitle file.
4. Click **Play** to start playback.
5. Subtitles will appear automatically based on the audio timestamp.

## Code Structure
- `src/gui/mobile_player.h/cpp`: Main widget for the player.
- `src/core/srt_parser.h/cpp`: SRT file parser.
