# 如何生成 Android APK 安装包

为了让您能够直接在手机上安装使用，我们需要生成一个独立的 `.apk` 文件。由于当前开发环境缺少 Android 构建工具（SDK/NDK），我们为您准备了**自动化构建方案**。

## 方案一：使用 GitHub Actions 自动生成（推荐）

这是获取"直接下载软件"最简单的方法，无需您本地配置任何环境。

1. **上传代码**：将本项目代码上传到您的 GitHub 仓库。
2. **启用 Actions**：在 GitHub 仓库页面，点击 "Actions" 标签页，确保 "Android Build" 工作流已启用。
3. **自动构建**：
   - 当您推送代码时，GitHub 会自动开始构建。
   - 构建过程通常需要 5-10 分钟。
4. **下载 APK**：
   - 构建完成后，点击该次构建记录。
   - 在页面底部的 **Artifacts** 区域，您会看到 `YUVPlayer-APK`。
   - 点击下载，解压后即可获得 `.apk` 文件。
5. **安装**：
   - 将 `.apk` 文件发送到手机。
   - 点击安装（无需配置 Qt，即装即用）。

## 方案二：本地构建（如果您有开发环境）

如果您想在本地电脑生成 APK，需要安装以下工具：
- **JDK 17** 或更高版本
- **Android SDK** (Command line tools, Platform tools)
- **Android NDK** (建议版本 r25b 或更高)
- **Qt for Android** (在 Qt Maintenance Tool 中勾选 Android ARM64-v8a)

### 构建步骤：
1. 打开 Qt Creator。
2. 配置 Android 构建套件（在工具 -> 选项 -> 设备 -> Android 中设置 SDK/NDK 路径）。
3. 打开本项目 `CMakeLists.txt`。
4. 选择 **Android Qt 6.x Clang arm64-v8a** 套件。
5. 选择 **Release** 模式。
6. 点击 **构建 (Build)**。
7. 构建成功后，APK 文件通常位于构建目录的 `android-build` 文件夹下。

## 关于配置文件
为了支持 APK 打包，我们已经为您自动生成了以下关键文件：
- `android/AndroidManifest.xml`: Android 应用清单文件，定义了权限和图标。
- `.github/workflows/android.yml`: GitHub Actions 自动化构建脚本。
