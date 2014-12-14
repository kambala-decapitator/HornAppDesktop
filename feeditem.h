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
        timestamp = dic["dtc"].toString().toInt();
        id = dic["id"].toString().toInt();
        message = dic["message"].toString();
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

typedef QList<TextItem *> TextItemList;

#endif // FEEDITEM_H
