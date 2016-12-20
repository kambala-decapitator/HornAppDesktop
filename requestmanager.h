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
class QIODevice;

class RequestManager : public QObject
{
    Q_OBJECT

public:
    static QString userHashIdentifier, token;
    static QString nickname;
    static int maxCategories;

    static double ipLatitude, ipLongitude;
    static bool hasIpGeo() { return ipLatitude || ipLongitude; }

    static RequestManager &instance()
    {
        static RequestManager obj;
        return obj;
    }

    void init(bool geoSourceUnavailable);

    QList<QString> categories() const { return _categoriesMap.values(); }
    QString categoryNameFromId(const QString &categoryId) { return _categoriesMap.value(categoryId, categoryId); }
    QString categoryIdFromName(const QString &categoryName) { return _categoriesMap.key(categoryName); }

    void requestPostsWithRequestPart(const QString &requestPart, FeedLambda callback, quint32 postIdForOlderFeed = 0);
    void requestComments(quint32 postId, FeedLambda callback, quint32 commentIdForOlderPosts = 0);
    void postComment(quint32 postId, const QString &comment, quint32 recipientCommentId, SuccessLambda callback);
    void createPost(const QString &message, const QStringList &tags, double latitude, double longitude, quint32 imageId, SuccessLambda callback);
    void uploadImage(QIODevice *device, std::function<void(const QJsonObject &)> callback);
    void requestNotifications(FeedLambda callback);
    void requestPostWithId(quint32 postId, std::function<void(FeedItem *)> callback);
    void markNotificationsRead(const QList<quint32> &ids);
    void requestCommentsVotes(const QList<quint32> &ids, std::function<void(const QHash<decltype(CommentItem::id), bool> &)> callback);
    void changeCommentVote(quint32 commentId, bool deleteVote, bool upvote, SuccessLambda callback);
    void changePostVote(quint32 postId, bool deleteVote, bool upvote, SuccessLambda callback);

signals:
    void nicknameChanged(const QString &nickname);

private slots:
    void updateUserInfo()
    {
        requestAuth();
        requestUserInfo();
        requestCategories();

        if (_requestIpGeo)
            requestGeoInfo();
    }

private:
    explicit RequestManager(QObject *parent = 0);

    void requestAuth();
    void requestUserInfo();
    void requestCategories();
    void requestGeoInfo();

    void changeVote(const QString &urlPart, bool deleteVote, bool upvote, SuccessLambda callback);

    void patchRequest(const QString &urlPart, const QByteArray &data, SuccessLambda callback = [](bool){});
    QNetworkRequest requestFromUrlParts(const QString &urlPart, bool get = true, const QString &urlJsonText = QString());

    FeedItem *feedItemFromJson(const QJsonObject &jsonObj);

    static QJsonArray arrayFromReply(QNetworkReply *reply);
    static QByteArray dataFromJsonObj(const QJsonObject &jsonObj);
    static QString arrayRequestParam(const QList<quint32> &ids);

private:
    QNetworkAccessManager *_qnam;
    QMap<QString, QString> _categoriesMap;
    bool _requestIpGeo = false;
};

#endif // REQUESTMANAGER_H
