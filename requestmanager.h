#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>

#include <QNetworkRequest>

#include <functional>

#include "feeditem.h"

typedef std::function<void(const QList<FeedItem> &)> FeedLambda;

class QNetworkAccessManager;

class RequestManager : public QObject
{
    Q_OBJECT

public:
    static RequestManager &instance()
    {
        static RequestManager obj;
        return obj;
    }

    void sendNewPostsRequest(FeedLambda callback);

signals:

public slots:

private:
    explicit RequestManager(QObject *parent = 0);
    RequestManager(RequestManager &) {}

    void sendAuthRequest();
    void sendUserRequest();
    void sendGeoRequest();

    QNetworkRequest requestFromUrlParts(const QString &urlPart, const QString &urlJsonText = QString());

private:
    QNetworkAccessManager *_qnam;
};

#endif // REQUESTMANAGER_H
