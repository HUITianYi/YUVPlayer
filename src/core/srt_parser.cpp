#include "srt_parser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

QVector<SubtitleItem> SrtParser::parse(const QString& filePath) {
    QVector<SubtitleItem> subtitles;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open SRT file:" << filePath;
        return subtitles;
    }

    QTextStream in(&file);
    // Ensure UTF-8 usually, but try to detect if needed. Assuming UTF-8 for now.
    in.setEncoding(QStringConverter::Utf8);

    SubtitleItem currentItem;
    bool expectingIndex = true;
    bool expectingTime = false;
    bool expectingText = false;

    // Regex for time line: 00:00:00,498 --> 00:00:02,827
    QRegularExpression timeRegex("(\\d{2}:\\d{2}:\\d{2},\\d{3})\\s-->\\s(\\d{2}:\\d{2}:\\d{2},\\d{3})");

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            if (expectingText && !currentItem.text.isEmpty()) {
                subtitles.append(currentItem);
                currentItem = SubtitleItem();
                expectingIndex = true;
                expectingTime = false;
                expectingText = false;
            }
            continue;
        }

        if (expectingIndex) {
            bool ok;
            int index = line.toInt(&ok);
            if (ok) {
                currentItem.index = index;
                expectingIndex = false;
                expectingTime = true;
            }
            continue;
        }

        if (expectingTime) {
            QRegularExpressionMatch match = timeRegex.match(line);
            if (match.hasMatch()) {
                currentItem.startTime = parseTime(match.captured(1));
                currentItem.endTime = parseTime(match.captured(2));
                expectingTime = false;
                expectingText = true;
            } else {
                // If time format is wrong, reset or skip? 
                // Let's assume structure is mostly correct or reset.
                expectingIndex = true; 
                expectingTime = false;
            }
            continue;
        }

        if (expectingText) {
            if (!currentItem.text.isEmpty()) {
                currentItem.text += "\n";
            }
            currentItem.text += line;
        }
    }
    
    // Add the last item if exists
    if (expectingText && !currentItem.text.isEmpty()) {
        subtitles.append(currentItem);
    }

    return subtitles;
}

qint64 SrtParser::parseTime(const QString& timeStr) {
    // Format: HH:mm:ss,zzz
    // Example: 00:00:00,498
    QString format = "HH:mm:ss,zzz";
    QTime time = QTime::fromString(timeStr, format);
    if (!time.isValid()) return 0;
    
    // We need total milliseconds from start of day, assuming duration < 24h
    // But better to parse manually if needed for >24h, though QTime handles wrapping.
    // Actually QTime(0,0,0).msecsTo(time) works.
    return QTime(0, 0, 0).msecsTo(time);
}
