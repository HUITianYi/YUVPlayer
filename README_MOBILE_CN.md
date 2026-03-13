# 移动端音频播放器 (WAV + SRT)

这是一个基于 Qt 的移动端风格音频播放器，支持播放 .wav 格式音频并同步显示 .srt 字幕。

## 功能特点
- **音频播放**: 支持 .wav 以及 Qt Multimedia 支持的其他音频格式。
- **字幕显示**: 解析 .srt 字幕文件，并根据音频播放进度自动显示对应字幕。
- **触控界面**: 大按钮设计，适合手机端操作。

## 如何编译与运行

### 1. 桌面端预览 (Windows/macOS/Linux)
如果您只是想在电脑上测试功能：
1. 使用 Qt Creator 打开项目根目录下的 `CMakeLists.txt`。
2. 选择您的桌面开发套件 (Desktop Kit)。
3. 点击 **构建 (Build)** 和 **运行 (Run)**。
4. 程序启动后：
   - 点击 **Open .wav** 选择您的音频文件。
   - 点击 **Open .srt** 选择配套的字幕文件。
   - 点击 **Play** 开始播放。

### 2. 部署到 Android 手机
如果您想在手机上运行：
1. 确保已安装 **Qt for Android** (通过 Qt Maintenance Tool 安装)。
2. 配置好 **Android SDK** 和 **NDK** (在 Qt Creator 的工具 -> 选项 -> 设备 -> Android 中配置)。
3. 在 Qt Creator 中打开项目。
4. 在左侧栏选择 **Projects (项目)**，添加 **Android** 构建套件 (Kit)。
5. 连接您的 Android 手机，并开启 **USB 调试** 模式。
6. 选择 Android Kit 并点击 **运行**，程序将自动安装到手机上。

## 使用说明
1. **加载音频**: 点击界面上的 "Open .wav" 按钮，从文件系统中选择音频文件。
2. **加载字幕**: 点击 "Open .srt" 按钮，选择对应的字幕文件。
3. **播放/暂停**: 点击底部的 "Play/Pause" 大按钮控制播放。
4. **进度控制**: 拖动进度条可以跳转播放位置，字幕会自动同步。

## 代码结构
- `src/gui/mobile_player.h/cpp`: 播放器主界面逻辑。
- `src/core/srt_parser.h/cpp`: 字幕解析器。
- `src/main.cpp`: 程序入口，目前已设置为启动移动端界面。
