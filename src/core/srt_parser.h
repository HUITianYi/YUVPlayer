#ifndef SRT_PARSER_H
#define SRT_PARSER_H

#include <QString>
#include <QVector>
#include <QTime>

struct SubtitleItem {
    int index;
    qint64 startTime; // milliseconds
    qint64 endTime;   // milliseconds
    QString text;
};

class SrtParser {
public:
    static QVector<SubtitleItem> parse(const QString& filePath);
    
private:
    static qint64 parseTime(const QString& timeStr);
};

#endif // SRT_PARSER_H
