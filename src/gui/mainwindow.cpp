#include "mainwindow.h"
#include "videowidget.h"
#include "yuv_reader.h"
#include "yuv_converter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_yuvReader(nullptr), m_rgbBuffer(nullptr), m_isPlaying(false), m_frameCount(0)
{
    setupUi();

    m_playTimer = new QTimer(this);
    connect(m_playTimer, &QTimer::timeout, this, &MainWindow::processNextFrame);

    m_perfTimer = new QElapsedTimer();
    
    // FPS Timer: updates every 1 second
    m_fpsTimer = new QTimer(this);
    connect(m_fpsTimer, &QTimer::timeout, this, &MainWindow::updateFPS);
    m_fpsTimer->start(1000);

    m_currentMode = MODE_SCALAR;
}

MainWindow::~MainWindow()
{
    cleanupYuv();
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 顶部控件
    QHBoxLayout *topLayout = new QHBoxLayout();

    m_btnOpen = new QPushButton("打开 YUV", this);
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::openFile);

    m_spinWidth = new QSpinBox(this);
    m_spinWidth->setRange(1, 8192);
    m_spinWidth->setValue(352); // CIF 默认值
    m_spinWidth->setPrefix("宽: ");

    m_spinHeight = new QSpinBox(this);
    m_spinHeight->setRange(1, 8192);
    m_spinHeight->setValue(288); // CIF 默认值
    m_spinHeight->setPrefix("高: ");

    m_spinFPS = new QSpinBox(this);
    m_spinFPS->setRange(1, 120);
    m_spinFPS->setValue(30);
    m_spinFPS->setPrefix("基准帧率: ");
    connect(m_spinFPS, QOverload<int>::of(&QSpinBox::valueChanged), [this](int fps)
            {
        changePlaybackRate(); // 更新定时器间隔
    });

    // 倍速选择
    m_comboRate = new QComboBox(this);
    m_comboRate->addItem("0.5x", 0.5);
    m_comboRate->addItem("1.0x", 1.0);
    m_comboRate->addItem("1.5x", 1.5);
    m_comboRate->addItem("2.0x", 2.0);
    m_comboRate->addItem("4.0x", 4.0);
    m_comboRate->setCurrentIndex(1); // 默认 1.0x
    connect(m_comboRate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changePlaybackRate);

    m_btnPlay = new QPushButton("播放", this);
    m_btnPlay->setCheckable(true);
    m_btnPlay->setEnabled(false);
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::playPause);

    topLayout->addWidget(m_btnOpen);
    topLayout->addWidget(m_spinWidth);
    topLayout->addWidget(m_spinHeight);
    topLayout->addWidget(m_spinFPS);
    topLayout->addWidget(new QLabel("倍速:"));
    topLayout->addWidget(m_comboRate);
    topLayout->addWidget(m_btnPlay);

    // 控制区域：快退 - 帧数信息 - 快进
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    m_btnBackward = new QPushButton("<< 后退15帧", this);
    m_btnBackward->setEnabled(false);
    connect(m_btnBackward, &QPushButton::clicked, this, &MainWindow::seekBackward);

    m_lblFrameInfo = new QLabel("0/0", this);
    m_lblFrameInfo->setAlignment(Qt::AlignCenter);
    m_lblFrameInfo->setMinimumWidth(100);

    m_btnForward = new QPushButton("快进15帧 >>", this);
    m_btnForward->setEnabled(false);
    connect(m_btnForward, &QPushButton::clicked, this, &MainWindow::seekForward);

    controlLayout->addStretch();
    controlLayout->addWidget(m_btnBackward);
    controlLayout->addWidget(m_lblFrameInfo);
    controlLayout->addWidget(m_btnForward);
    controlLayout->addStretch();

    // 模式选择
    QHBoxLayout *modeLayout = new QHBoxLayout();
    m_radScalar = new QRadioButton("标量 (C)", this);
    m_radSSE = new QRadioButton("SSE (SIMD)", this);
    m_radMT = new QRadioButton("多线程", this);

    m_radScalar->setChecked(true);

    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(m_radScalar);
    group->addButton(m_radSSE);
    group->addButton(m_radMT);

    connect(m_radScalar, &QRadioButton::toggled, this, &MainWindow::updateMode);
    connect(m_radSSE, &QRadioButton::toggled, this, &MainWindow::updateMode);
    connect(m_radMT, &QRadioButton::toggled, this, &MainWindow::updateMode);

    modeLayout->addWidget(new QLabel("模式:"));
    modeLayout->addWidget(m_radScalar);
    modeLayout->addWidget(m_radSSE);
    modeLayout->addWidget(m_radMT);
    modeLayout->addStretch();

    // 视频区域
    m_videoWidget = new VideoWidget(this);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setMinimumSize(352, 288);

    // 状态栏
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_lblStatus = new QLabel("就绪", this);
    m_lblFPS = new QLabel("FPS: 0", this);
    m_lblTime = new QLabel("耗时: 0 ms", this);

    statusLayout->addWidget(m_lblStatus);
    statusLayout->addStretch();
    statusLayout->addWidget(m_lblTime);
    statusLayout->addWidget(m_lblFPS);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(modeLayout);
    mainLayout->addWidget(m_videoWidget);
    mainLayout->addLayout(statusLayout);
}

void MainWindow::updateMode()
{
    if (m_radScalar->isChecked())
        m_currentMode = MODE_SCALAR;
    else if (m_radSSE->isChecked())
        m_currentMode = MODE_SSE;
    else if (m_radMT->isChecked())
        m_currentMode = MODE_MT;
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开 YUV 文件", "", "视频文件 (*.yuv *.y4m);;YUV 文件 (*.yuv);;Y4M 文件 (*.y4m);;所有文件 (*)");
    if (fileName.isEmpty())
        return;

    cleanupYuv();

    int width = m_spinWidth->value();
    int height = m_spinHeight->value();

    // 需要正确地将 QString 转换为 const char*
    QByteArray ba = fileName.toLocal8Bit();
    const char *c_str = ba.data();

    m_yuvReader = yuv_reader_create(c_str, width, height);
    if (!m_yuvReader)
    {
        QMessageBox::critical(this, "错误", "无法打开 YUV 文件或分配内存。");
        return;
    }

    // 如果是 Y4M，更新界面上的宽/高显示
    if (m_yuvReader->is_y4m) {
        m_spinWidth->blockSignals(true);
        m_spinHeight->blockSignals(true);
        m_spinWidth->setValue(m_yuvReader->width);
        m_spinHeight->setValue(m_yuvReader->height);
        m_spinWidth->blockSignals(false);
        m_spinHeight->blockSignals(false);
        
        // 更新本地变量以正确分配 RGB buffer
        width = m_yuvReader->width;
        height = m_yuvReader->height;
    }

    m_lblFrameInfo->setText(QString("0/%1").arg(m_yuvReader->total_frames));
    m_btnForward->setEnabled(true);
    m_btnBackward->setEnabled(true);

    // 强制调用一次 changePlaybackRate 以确保定时器间隔正确初始化（虽然此时并未播放）
    changePlaybackRate();

    // 分配 RGB 缓冲区
    // 每个像素 3 字节
    m_rgbBuffer = (uint8_t *)malloc(width * height * 3);
    if (!m_rgbBuffer)
    {
        cleanupYuv();
        QMessageBox::critical(this, "错误", "无法分配 RGB 缓冲区。");
        return;
    }

    m_btnPlay->setEnabled(true);
    m_lblStatus->setText(QString("已加载: %1 (%2x%3)").arg(fileName).arg(width).arg(height));

    // 显示第一帧
    processNextFrame();
}

void MainWindow::cleanupYuv()
{
    if (m_yuvReader)
    {
        yuv_reader_destroy(m_yuvReader);
        m_yuvReader = nullptr;
    }
    if (m_rgbBuffer)
    {
        free(m_rgbBuffer);
        m_rgbBuffer = nullptr;
    }
    m_btnPlay->setEnabled(false);
    m_isPlaying = false;
    m_playTimer->stop();
    m_btnPlay->setText("播放");
    m_btnPlay->setChecked(false);
    m_frameCount = 0;
    
    m_lblFrameInfo->setText("0/0");
    m_btnForward->setEnabled(false);
    m_btnBackward->setEnabled(false);
}

void MainWindow::playPause()
{
    if (!m_yuvReader)
        return;

    if (m_isPlaying)
    {
        m_playTimer->stop();
        m_btnPlay->setText("播放");
        m_isPlaying = false;
    }
    else
    {
        m_isPlaying = true; // 先设置状态
        m_btnPlay->setText("暂停");
        
        // 确保使用当前设置的 FPS 和倍速启动定时器
        changePlaybackRate(); 
        
        // 强制启动（双重保险，因为 changePlaybackRate 现在会检查 m_isPlaying）
        if (!m_playTimer->isActive()) {
             m_playTimer->start();
        }
    }
}

void MainWindow::changePlaybackRate()
{
    if (!m_yuvReader) return;
    
    double rate = m_comboRate->currentData().toDouble();
    int baseFPS = m_spinFPS->value();
    // 保护：如果 baseFPS 为 0，设为 1
    if (baseFPS <= 0) baseFPS = 1;

    int interval = 1000 / (baseFPS * rate);
    if (interval < 1) interval = 1;

    // 不论当前是否在播放，都更新定时器间隔，但只在播放时启动
    m_playTimer->setInterval(interval); 
    
    if (m_isPlaying) {
        m_playTimer->start();
    }
}

void MainWindow::seekForward()
{
    if (!m_yuvReader) return;

    int current = m_yuvReader->frame_index;
    int target = current + 15;
    if (target >= m_yuvReader->total_frames) target = m_yuvReader->total_frames - 1;
    if (target < 0) target = 0; 

    yuv_reader_seek(m_yuvReader, target);
    
    // 立即刷新画面（即使在暂停时）
    if (!m_isPlaying) {
        // 为了显示 seek 后的那一帧，我们需要再调一次 next_frame 或者 seek 后读
        // yuv_reader_seek 只是定位了文件指针
        // processNextFrame 会读取并显示，且会 frame_index++
        // 如果我们想停在 target 帧，processNextFrame 会显示 target 并停在 target+1
        // 这符合逻辑
        processNextFrame();
    } else {
        // 播放中，不需要额外操作，下一帧定时器触发时自然会读到
        // 只是更新一下 Label
        m_lblFrameInfo->setText(QString("%1/%2").arg(m_yuvReader->frame_index).arg(m_yuvReader->total_frames));
    }
}

void MainWindow::seekBackward()
{
    if (!m_yuvReader) return;

    int current = m_yuvReader->frame_index;
    int target = current - 15;
    if (target < 0) target = 0;

    yuv_reader_seek(m_yuvReader, target);
    
    if (!m_isPlaying) {
        processNextFrame();
    } else {
        m_lblFrameInfo->setText(QString("%1/%2").arg(m_yuvReader->frame_index).arg(m_yuvReader->total_frames));
    }
}

void MainWindow::processNextFrame()
{
    if (!m_yuvReader)
        return;

    // 读取 YUV
    if (!yuv_reader_next_frame(m_yuvReader))
    {
        // 循环播放
        yuv_reader_seek(m_yuvReader, 0);
        if (!yuv_reader_next_frame(m_yuvReader))
            return;
    }

    m_lblFrameInfo->setText(QString("%1/%2").arg(m_yuvReader->frame_index).arg(m_yuvReader->total_frames));

    int width = m_yuvReader->current_frame.width;
    int height = m_yuvReader->current_frame.height;

    m_perfTimer->restart();

    // 转换
    switch (m_currentMode)
    {
    case MODE_SCALAR:
        yuv420_to_rgb_scalar(m_yuvReader->current_frame.y_data,
                             m_yuvReader->current_frame.u_data,
                             m_yuvReader->current_frame.v_data,
                             m_rgbBuffer, width, height);
        break;
    case MODE_SSE:
        yuv420_to_rgb_sse(m_yuvReader->current_frame.y_data,
                          m_yuvReader->current_frame.u_data,
                          m_yuvReader->current_frame.v_data,
                          m_rgbBuffer, width, height);
        break;
    case MODE_MT:
        // 默认使用 4 个线程
        yuv420_to_rgb_mt(m_yuvReader->current_frame.y_data,
                         m_yuvReader->current_frame.u_data,
                         m_yuvReader->current_frame.v_data,
                         m_rgbBuffer, width, height, 4);
        break;
    }

    qint64 elapsed = m_perfTimer->nsecsElapsed();
    double ms = elapsed / 1000000.0;

    m_lblTime->setText(QString("处理耗时: %1 ms").arg(ms, 0, 'f', 2));

    // 显示
    // QImage::Format_RGB888 期望数据是打包的 RGB
    QImage img(m_rgbBuffer, width, height, width * 3, QImage::Format_RGB888);
    m_videoWidget->updateFrame(img);
    
    // FPS counting
    m_frameCount++;
}

void MainWindow::updateFPS()
{
    m_lblFPS->setText(QString("FPS: %1").arg(m_frameCount));
    m_frameCount = 0;
}
