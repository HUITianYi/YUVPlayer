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

private:
    void setupUi();
    void cleanupYuv();

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

    QRadioButton *m_radScalar;
    QRadioButton *m_radSSE;
    QRadioButton *m_radMT;

    QLabel *m_lblStatus;
    QLabel *m_lblFPS;
    QLabel *m_lblTime;
    QLabel *m_lblFrameInfo; // 显示当前帧/总帧数

    QTimer *m_playTimer;
    QElapsedTimer *m_perfTimer;
    
    // FPS counting
    QTimer *m_fpsTimer;
    int m_frameCount;

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
