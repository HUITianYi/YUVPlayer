# High-Performance YUV Video Player

![YUV Player Screenshot](screenshot.png)

A high-performance YUV video player developed in C/C++ with Qt, featuring SSE optimization and multi-threading support.

## Features

- **Format Support**: Plays raw YUV420P (`.yuv`) and YUV4MPEG2 (`.y4m`) files.
- **High Performance**:
  - **Scalar**: Optimized integer arithmetic for color conversion.
  - **SSE SIMD**: Utilizes SSE2 instructions for vectorized pixel processing.
  - **Multi-threading**: Parallel processing using Windows threading API.
- **Playback Control**:
  - Play/Pause
  - Seek Forward/Backward (±15 frames)
  - Variable Playback Speed (0.5x, 1.0x, 1.5x, 2.0x, 4.0x)
- **GUI**: Modern interface built with Qt 6.
- **Benchmarks**: Includes a benchmarking tool to compare performance across different modes.

## Requirements

- **Compiler**: GCC (MinGW) or MSVC with C++17 support.
- **CMake**: Version 3.20 or higher.
- **Qt**: Version 6.x (tested with Qt 6.10.2).

## Build Instructions

### Windows (MinGW)

1.  Clone the repository:
    ```bash
    git clone https://github.com/your-username/YUVPlayer.git
    cd YUVPlayer
    ```

2.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```

3.  Configure and build:
    ```bash
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    ```

4.  Run the player:
    ```bash
    windeployqt YUVPlayer.exe
    ./YUVPlayer.exe
    ```

## Performance

Tested on [Your CPU Model] @ 1080p Resolution:

| Mode | Time per Frame | Speedup |
|------|----------------|---------|
| Scalar | ~6.61 ms | 1.00x |
| SSE | ~4.24 ms | 1.56x |
| Multi-threaded | ~1.44 ms | 4.60x |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

**HTea** - [GitHub Profile](https://github.com/your-username)
