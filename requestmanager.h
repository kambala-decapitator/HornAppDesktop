#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>

#include <QNetworkRequest>

#include <functional>

#include "feeditem.h"

typedef std::function<void(const TextItemList &)> FeedLambda;
typedef std::function<void(bool)> SuccessLambda;

class QNetworkAccessManager;
class QNetworkReply;

class RequestManager : public QObject
{
    Q_OBJECT

public:
    static QString userHashIdentifier;

    static RequestManager &instance()
    {
        static RequestManager obj;
        return obj;
    }

    const QString &userNickname() const { return _nickname; }

    void requestPostsWithRequestPart(const QString &requestPart, FeedLambda callback, quint32 postIdForOlderFeed = 0);
    void requestComments(quint32 postId, FeedLambda callback, quint32 commentIdForOlderPosts = 0);
    void postComment(quint32 postId, const QString &comment, quint32 recipientCommentId, SuccessLambda callback);
    void createPost(const QString &message, const QStringList &tags, double latitude, double longitude, SuccessLambda callback);
    void requestNotifications(FeedLambda callback);
    void requestPostWithId(quint32 postId, std::function<void(FeedItem *)> callback);
    void markNotificationsRead(const QList<quint32> &ids);
    void requestCommentsVotes(const QList<quint32> &ids, std::function<void(const QHash<decltype(CommentItem::id), bool> &)> callback);
    void changeCommentVote(quint32 commentId, bool deleteVote, bool upvote, SuccessLambda callback);

signals:
    void nicknameChanged(const QString &nickname);

private slots:
    void updateUserInfo()
    {
        requestAuth();
        requestUserInfo();
//        requestGeoInfo();
    }

private:
    explicit RequestManager(QObject *parent = 0);
    RequestManager(RequestManager &) {}

    void requestAuth();
    void requestUserInfo();
    void requestGeoInfo();

    void patchRequest(const QString &urlPart, const QByteArray &data, SuccessLambda callback = [](bool){});
    QNetworkRequest requestFromUrlParts(const QString &urlPart, bool get = true, const QString &urlJsonText = QString());

    static FeedItem *feedItemFromJson(const QJsonObject &jsonObj);
    static QJsonArray arrayFromReply(QNetworkReply *reply);
    static QByteArray dataFromJsonObj(const QJsonObject &jsonObj);
    static QString arrayRequestParam(const QList<quint32> &ids);

private:
    QNetworkAccessManager *_qnam;
    QString _nickname;
};

#endif // REQUESTMANAGER_H
