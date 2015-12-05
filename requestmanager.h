#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>

#include <QNetworkRequest>

#include <functional>

#include "feeditem.h"

typedef std::function<void(const TextItemList &)> FeedLambda;

class QNetworkAccessManager;
class QNetworkReply;

class RequestManager : public QObject
{
    Q_OBJECT

public:
    static RequestManager &instance()
    {
        static RequestManager obj;
        return obj;
    }

    const QString &userNickname() const { return _nickname; }

    void requestNewPosts(FeedLambda callback, quint32 postIdForOlderFeed = 0);
    void requestComments(quint32 postId, FeedLambda callback);
    void postComment(quint32 postId, const QString &comment, quint32 recipientCommentId, std::function<void(bool)> callback);

signals:

public slots:

private:
    explicit RequestManager(QObject *parent = 0);
    RequestManager(RequestManager &) {}

    void requestAuth();
    void requestUserInfo();
    void requestGeoInfo();

    QNetworkRequest requestFromUrlParts(const QString &urlPart, bool get = true, const QString &urlJsonText = QString());
    static QJsonArray arrayFromReply(QNetworkReply *reply);
    static QByteArray dataFromJsonObj(const QJsonObject &jsonObj);

private:
    QNetworkAccessManager *_qnam;
    QString _nickname;
};

#endif // REQUESTMANAGER_H
