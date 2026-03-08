#include "videowidget.h"
#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent); // 优化：不透明绘制
    setAutoFillBackground(false);
}

void VideoWidget::updateFrame(const QImage &frame)
{
    if (frame.isNull())
        return;
    m_currentFrame = frame.copy(); // 创建副本
    update();                      // 触发重绘
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (!m_currentFrame.isNull())
    {
        painter.drawImage(rect(), m_currentFrame, m_currentFrame.rect());
        
        // Draw Watermark
        painter.setPen(Qt::yellow);
        QFont font = painter.font();
        font.setPixelSize(20);
        font.setBold(true);
        painter.setFont(font);
        
        // Bottom-right corner
        QString text = "HTea";
        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(text);
        int textHeight = fm.height();
        
        painter.drawText(width() - textWidth - 10, height() - 10, text);
    }
    else
    {
        painter.fillRect(rect(), Qt::black);
    }
}
