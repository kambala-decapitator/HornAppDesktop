#ifndef FEEDITEM_H
#define FEEDITEM_H

#include <QString>
#include <QPoint>

struct FeedItem
{
    QString background;
    QPoint coordinates;
    quint32 timestamp;
    quint32 id;
    QString message;
    quint32 comments;
    qint32 reputation;
};

#endif // FEEDITEM_H
