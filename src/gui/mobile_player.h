#ifndef MOBILE_PLAYER_H
#define MOBILE_PLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTime>
#include "srt_parser.h"

class MobilePlayerWidget : public QWidget {
    Q_OBJECT
public:
    explicit MobilePlayerWidget(QWidget *parent = nullptr);
    ~MobilePlayerWidget();

private slots:
    void openAudio();
    void openSubtitle();
    void playPause();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSliderMoved(int position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void updateSubtitle(qint64 position);
    QString formatTime(qint64 ms);

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    
    QLabel *m_lblTitle;
    QLabel *m_lblSubtitle;
    QPushButton *m_btnPlay;
    QPushButton *m_btnOpenAudio;
    QPushButton *m_btnOpenSrt;
    QSlider *m_slider;
    QLabel *m_lblTime;
    
    QVector<SubtitleItem> m_subtitles;
    int m_currentSubtitleIndex;
    bool m_isSliderPressed;
};

#endif // MOBILE_PLAYER_H
