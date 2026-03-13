#include "mobile_player.h"
#include <QStyle>

MobilePlayerWidget::MobilePlayerWidget(QWidget *parent)
    : QWidget(parent), m_currentSubtitleIndex(-1), m_isSliderPressed(false)
{
    // Initialize Player
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    
    connect(m_player, &QMediaPlayer::positionChanged, this, &MobilePlayerWidget::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MobilePlayerWidget::onDurationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MobilePlayerWidget::onMediaStatusChanged);

    // UI Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 40, 20, 40);

    // Title
    m_lblTitle = new QLabel("Audio Player", this);
    m_lblTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_lblTitle->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_lblTitle->setFont(titleFont);
    mainLayout->addWidget(m_lblTitle);

    // Subtitle Display Area
    m_lblSubtitle = new QLabel("No Subtitle", this);
    m_lblSubtitle->setAlignment(Qt::AlignCenter);
    m_lblSubtitle->setWordWrap(true);
    QFont subFont = m_lblSubtitle->font();
    subFont.setPointSize(16);
    m_lblSubtitle->setFont(subFont);
    m_lblSubtitle->setStyleSheet("color: #333333; background-color: #f0f0f0; border-radius: 10px; padding: 10px;");
    m_lblSubtitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(m_lblSubtitle, 1); // Stretch factor 1

    // Controls
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_btnOpenAudio = new QPushButton("Open .wav", this);
    m_btnOpenSrt = new QPushButton("Open .srt", this);
    m_btnOpenAudio->setMinimumHeight(50);
    m_btnOpenSrt->setMinimumHeight(50);
    connect(m_btnOpenAudio, &QPushButton::clicked, this, &MobilePlayerWidget::openAudio);
    connect(m_btnOpenSrt, &QPushButton::clicked, this, &MobilePlayerWidget::openSubtitle);
    fileLayout->addWidget(m_btnOpenAudio);
    fileLayout->addWidget(m_btnOpenSrt);
    mainLayout->addLayout(fileLayout);

    // Slider & Time
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    m_lblTime = new QLabel("00:00 / 00:00", this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 0);
    connect(m_slider, &QSlider::sliderMoved, this, &MobilePlayerWidget::onSliderMoved);
    connect(m_slider, &QSlider::sliderPressed, [this]() { m_isSliderPressed = true; });
    connect(m_slider, &QSlider::sliderReleased, [this]() { 
        m_isSliderPressed = false; 
        m_player->setPosition(m_slider->value());
    });
    
    sliderLayout->addWidget(m_lblTime);
    sliderLayout->addWidget(m_slider);
    mainLayout->addLayout(sliderLayout);

    // Play Button
    m_btnPlay = new QPushButton("Play", this);
    m_btnPlay->setCheckable(true);
    m_btnPlay->setMinimumHeight(60);
    QFont btnFont = m_btnPlay->font();
    btnFont.setPointSize(14);
    m_btnPlay->setFont(btnFont);
    connect(m_btnPlay, &QPushButton::clicked, this, &MobilePlayerWidget::playPause);
    mainLayout->addWidget(m_btnPlay);
}

MobilePlayerWidget::~MobilePlayerWidget() {
}

void MobilePlayerWidget::openAudio() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Audio", "", "Audio Files (*.wav *.mp3);;All Files (*)");
    if (fileName.isEmpty()) return;

    m_player->setSource(QUrl::fromLocalFile(fileName));
    m_lblTitle->setText(QFileInfo(fileName).fileName());
    m_btnPlay->setText("Play");
    m_btnPlay->setChecked(false);
}

void MobilePlayerWidget::openSubtitle() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Subtitle", "", "Subtitle Files (*.srt);;All Files (*)");
    if (fileName.isEmpty()) return;

    m_subtitles = SrtParser::parse(fileName);
    m_currentSubtitleIndex = -1;
    m_lblSubtitle->setText("Subtitle Loaded");
}

void MobilePlayerWidget::playPause() {
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
        m_btnPlay->setText("Play");
    } else {
        m_player->play();
        m_btnPlay->setText("Pause");
    }
}

void MobilePlayerWidget::onPositionChanged(qint64 position) {
    if (!m_isSliderPressed) {
        m_slider->setValue(position);
    }
    
    qint64 duration = m_player->duration();
    m_lblTime->setText(QString("%1 / %2").arg(formatTime(position)).arg(formatTime(duration)));

    updateSubtitle(position);
}

void MobilePlayerWidget::onDurationChanged(qint64 duration) {
    m_slider->setRange(0, duration);
}

void MobilePlayerWidget::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        m_btnPlay->setText("Play");
        m_btnPlay->setChecked(false);
    }
}

void MobilePlayerWidget::onSliderMoved(int position) {
    // Just update UI time, seek happens on release
    qint64 duration = m_player->duration();
    m_lblTime->setText(QString("%1 / %2").arg(formatTime(position)).arg(formatTime(duration)));
}

void MobilePlayerWidget::updateSubtitle(qint64 position) {
    if (m_subtitles.isEmpty()) return;

    // Check if current subtitle is still valid
    if (m_currentSubtitleIndex >= 0 && m_currentSubtitleIndex < m_subtitles.size()) {
        const auto& item = m_subtitles[m_currentSubtitleIndex];
        if (position >= item.startTime && position <= item.endTime) {
            return; // Still in the same subtitle
        }
    }

    // Find new subtitle
    // Linear search is fine for typical subtitle counts, but let's optimize slightly
    // If we moved forward, search from current index
    // If we jumped back, search from start
    
    int startIndex = 0;
    if (m_currentSubtitleIndex >= 0 && position > m_subtitles[m_currentSubtitleIndex].startTime) {
        startIndex = m_currentSubtitleIndex;
    }

    bool found = false;
    for (int i = startIndex; i < m_subtitles.size(); ++i) {
        const auto& item = m_subtitles[i];
        if (position >= item.startTime && position <= item.endTime) {
            if (m_currentSubtitleIndex != i) {
                m_lblSubtitle->setText(item.text);
                m_currentSubtitleIndex = i;
            }
            found = true;
            break;
        }
        if (item.startTime > position) {
            break; // Since subtitles are sorted
        }
    }

    if (!found) {
        // Clear if no subtitle found
        if (m_currentSubtitleIndex != -1) {
            m_lblSubtitle->setText("");
            m_currentSubtitleIndex = -1;
        }
    }
}

QString MobilePlayerWidget::formatTime(qint64 ms) {
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000) % 60;
    int hours = (ms / 3600000);

    if (hours > 0)
        return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0'))
                                  .arg(minutes, 2, 10, QChar('0'))
                                  .arg(seconds, 2, 10, QChar('0'));
    
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0'))
                           .arg(seconds, 2, 10, QChar('0'));
}
