#ifndef FEEDITEM_H
#define FEEDITEM_H

#include <QString>
#include <QPoint>
#include <QJsonObject>

struct TextItem
{
    quint32 timestamp;
    quint32 id;
    QString message;
    qint32 reputation;

    void setupFromJson(const QJsonObject &dic)
    {
        timestamp = dic["dtc"].toString().toULongLong();
        id = dic["id"].toString().toULongLong();
        message = dic["message"].toString().trimmed();
        reputation = dic["svotes"].toString().toInt();
    }
};

struct FeedItem : public TextItem
{
    QString background;
    QPoint coordinates;
    quint32 comments;
};

struct CommentItem : public TextItem
{
    QString nickname, recipientNickname;
};

struct NotificationItem : public TextItem
{
    bool isRead;
    quint32 postId;
    quint32 commentId1, commentId2; // voted comment or answer and to which comment
    QString type;
};

typedef QList<TextItem *> TextItemList;

#endif // FEEDITEM_H
