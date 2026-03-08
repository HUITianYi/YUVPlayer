#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void updateFrame(const QImage &frame);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_currentFrame;
};

#endif // VIDEOWIDGET_H
