#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include "videowidget.h"
#include "yuv_common.h"
#include "yuv_converter.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void playPause();
    void processNextFrame();
    void updateMode();
    void updateFPS();
    void changePlaybackRate(); // 更改倍速
    void seekForward();        // 快进
    void seekBackward();       // 快退
    void toggleAutoMode(bool checked); // 切换自动模式

private:
    void setupUi();
    void cleanupYuv();
    void selectBestMode(); // 选择最佳模式

    VideoWidget *m_videoWidget;
    QPushButton *m_btnOpen;
    QPushButton *m_btnPlay;
    QSpinBox *m_spinWidth;
    QSpinBox *m_spinHeight;
    QSpinBox *m_spinFPS;
    
    // 倍速相关
    QComboBox *m_comboRate;

    // 快进快退按钮
    QPushButton *m_btnForward;
    QPushButton *m_btnBackward;

    // 自动选择模式
    QCheckBox *m_checkAuto;

    QRadioButton *m_radScalar;
    QRadioButton *m_radSSE;
    QRadioButton *m_radMT;

    QLabel *m_lblStatus;
    QLabel *m_lblFPS;
    QLabel *m_lblTime;
    QLabel *m_lblAvgTime; // 平均耗时
    QLabel *m_lblFrameInfo; // 显示当前帧/总帧数

    QTimer *m_playTimer;
    QElapsedTimer *m_perfTimer;
    
    // FPS counting
    QTimer *m_fpsTimer;
    int m_frameCount;

    // 平均耗时计算
    double m_totalTime;
    int m_processedFrames;

    YUVReader *m_yuvReader;
    uint8_t *m_rgbBuffer;
    bool m_isPlaying;

    enum ProcessMode
    {
        MODE_SCALAR,
        MODE_SSE,
        MODE_MT
    };
    ProcessMode m_currentMode;
};

#endif // MAINWINDOW_H
