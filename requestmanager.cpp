#include "requestmanager.h"

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <QApplication>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

static const QLatin1String kHornAppBaseUrl("http://app.hornapp.com/request/v1/");
static const QLatin1String kToken("1dea204df061332e3703f385f3203327839765260fac58f94202149fa3c6aefc");
static const int kPostsPerPage = 20;

QString RequestManager::userID("525c87a74549fa59bd41829815f024c21cc352fe6ba85aa047a3e1c73f53cf2f");

RequestManager::RequestManager(QObject *parent) : QObject(parent), _qnam(new QNetworkAccessManager)
{
    connect(_qnam, &QNetworkAccessManager::finished, [](QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << reply->errorString();
            qDebug() << reply->rawHeaderPairs();
        }
        reply->deleteLater();
    });

    requestAuth();
    requestUserInfo();
    requestGeoInfo();
}

void RequestManager::requestPostsWithRequestPart(const QString &requestPart, FeedLambda callback, quint32 postIdForOlderFeed)
{
    QJsonObject defaultCondition;
    defaultCondition["lang"] = "ru";
    defaultCondition["age"] = 1;

    QJsonObject conditions;
    conditions["0"] = defaultCondition;
    if (postIdForOlderFeed)
    {
        QJsonObject nextPageCondition;
        nextPageCondition["id"] = static_cast<qint64>(postIdForOlderFeed);
        conditions["5"] = nextPageCondition;
    }

    QJsonObject dic;
    dic["limit"] = kPostsPerPage;
    dic["conditions"] = conditions;

    auto reply = _qnam->get(requestFromUrlParts(requestPart + QLatin1String("/"), true, dataFromJsonObj(dic)));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        TextItemList result;
        if (reply->error() == QNetworkReply::NoError)
        {
            for (const auto &value : arrayFromReply(reply))
            {
                FeedItem *item = new FeedItem;
                auto dic = value.toObject();
                item->setupFromJson(dic);
                item->comments = dic["scomms"].toString().toInt();

                auto background = dic["bg"].toString();
                if (background.isEmpty())
                    background = dic["image"].toObject()["link"].toString();
                item->background = background;

                auto coordinates = dic["centroid"].toObject()["coordinates"].toArray();
                if (coordinates.size() == 2)
                    item->coordinates = QPoint(coordinates.at(0).toInt(), coordinates.at(1).toInt());

                result += item;
            }
        }
        callback(result);
    });
}

void RequestManager::requestComments(quint32 postId, FeedLambda callback)
{
    QJsonObject dic;
    dic["horn_id"] = QJsonValue(static_cast<qint64>(postId));

    _qnam->post(requestFromUrlParts(QLatin1String("Horn/Entry/"), false), dataFromJsonObj(dic));

    auto reply = _qnam->get(requestFromUrlParts(QString("Horn/%1/Comment/").arg(postId)));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        if (reply->error() == QNetworkReply::NoError)
        {
            TextItemList result;
            for (const auto &value : arrayFromReply(reply))
            {
                CommentItem *item = new CommentItem;
                auto dic = value.toObject();
                item->setupFromJson(dic);
                item->nickname = dic["nickname"].toString();
                item->recipientNickname = dic["reply_to_nickname"].toString();

                result += item;
            }
            callback(result);
        }
    });
}

void RequestManager::postComment(quint32 postId, const QString &comment, quint32 recipientCommentId, SuccessLambda callback)
{
    QJsonObject dic;
    dic["message"] = QJsonValue(comment);
    if (recipientCommentId)
        dic["parent_id"] = QJsonValue(static_cast<qint64>(recipientCommentId));

    auto reply = _qnam->post(requestFromUrlParts(QString("Horn/%1/Comment/").arg(postId), false), dataFromJsonObj(dic));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        auto response = reply->readAll(); // "{"Errors":{"ERROR_USER_CANT_POST":[]}}"
        qDebug() << response;
        callback(reply->error() == QNetworkReply::NoError && !response.isEmpty());
    });
}

void RequestManager::createPost(const QString &message, const QStringList &tags, double latitude, double longitude, SuccessLambda callback)
{
    QJsonObject dic;
    dic["message"] = message;
    dic["bg"] = "white:sample"; // let Horn choose image
    dic["lang"] = "ru";
    dic["hashtags"] = QJsonArray::fromStringList(tags);

    if (!qIsNaN(latitude) && !qIsNaN(longitude))
    {
        QJsonObject geo;
        geo["type"] = "Point";
        geo["coordinates"] = QJsonArray({longitude, latitude});
        dic["geometry"] = geo;
    }

    auto reply = _qnam->post(requestFromUrlParts("Horn/", false), dataFromJsonObj(dic));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        qDebug() << reply->readAll(); // "{"Errors":{"ERROR_USER_CANT_POST":[]}}"
        callback(reply->error() == QNetworkReply::NoError);
    });
}

// private

void RequestManager::requestAuth()
{
    auto buf = new QBuffer;
    buf->buffer() = "{}";
    auto deleteBufLambda = [buf]{ delete buf; };

    if (buf->open(QIODevice::ReadOnly))
        connect(_qnam->sendCustomRequest(requestFromUrlParts(QString("Auth/%1").arg(kToken)), "PATCH", buf), &QNetworkReply::finished, deleteBufLambda);
    else
        deleteBufLambda();
}

void RequestManager::requestUserInfo()
{
    auto reply = _qnam->get(requestFromUrlParts(QString("User/%1").arg(userID)));
    connect(reply, &QNetworkReply::finished, [reply, this]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto dic = QJsonDocument::fromJson(reply->readAll()).object();
            auto nickname = dic["nickname"].toString();
            qDebug() << nickname << "=" << dic["rep"].toString().toInt();
            _nickname = nickname;
        }
    });
}

void RequestManager::requestGeoInfo()
{
    _qnam->get(requestFromUrlParts(QLatin1String("IpGeo/1")));
}

QNetworkRequest RequestManager::requestFromUrlParts(const QString &urlPart, bool get, const QString &urlJsonText)
{
    auto urlString = kHornAppBaseUrl + QString("%1?token=%2").arg(urlPart, kToken);
    if (!urlJsonText.isEmpty())
        urlString += QLatin1String("&json=") + QString::fromUtf8(QUrl::toPercentEncoding(urlJsonText));

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
#ifdef Q_OS_MAC
    QString os = "Mac OS X 10.11.3";
#else
    QString os = "Windows 8.1 x64";
#endif
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2 (%3)").arg(qApp->applicationName(), qApp->applicationVersion(), os));
    if (!get)
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json; charset=utf-8"));
    return request;
}

QJsonArray RequestManager::arrayFromReply(QNetworkReply *reply)
{
    return QJsonDocument::fromJson(reply->readAll()).array();
}

QByteArray RequestManager::dataFromJsonObj(const QJsonObject &jsonObj)
{
    return QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
}
