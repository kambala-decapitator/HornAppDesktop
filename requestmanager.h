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

    void requestNewPosts(FeedLambda callback);
    void requestComments(quint32 postId, FeedLambda callback);

signals:

public slots:

private:
    explicit RequestManager(QObject *parent = 0);
    RequestManager(RequestManager &) {}

    void requestAuth();
    void requestUserInfo();
    void requestGeoInfo();

    QNetworkRequest requestFromUrlParts(const QString &urlPart, const QString &urlJsonText = QString());
    static QJsonArray arrayFromReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *_qnam;
};

#endif // REQUESTMANAGER_H
