#include "gui/mainwindow.h"
#include "gui/mobile_player.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置样式
    a.setStyle("Fusion");

    // 设置应用程序图标
    a.setWindowIcon(QIcon(":/icon.png"));

    // Uncomment below to run original YUV player
    // MainWindow w;
    // w.setWindowTitle("高性能 YUV 播放器 (C/C++/SSE/MT)");
    // w.setWindowIcon(QIcon(":/icon.png")); // 确保窗口图标也被设置
    // w.resize(800, 600);
    // w.show();

    // Use Mobile Player for WAV/SRT
    MobilePlayerWidget w;
    w.setWindowTitle("WAV & SRT Player");
    w.setWindowIcon(QIcon(":/icon.png"));
    w.resize(400, 700); // Mobile-like aspect ratio
    w.show();

    return a.exec();
}
