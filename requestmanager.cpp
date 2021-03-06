#include "requestmanager.h"

#include <QBuffer>
#include <QTimer>

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

static const QLatin1String kHornAppBaseUrl("http://app.hornapp.com/request/v1/"), HornUploadImageBaseUrl("http://upload.img.hornapp.com/request/v1/Horn/Image/");
static const int kPostsPerPage = 50, kCommentsPerPage = 100, UpdateUserInfoMsec = 1000 * 60 * 30;

QString RequestManager::userHashIdentifier;
QString RequestManager::token;
QString RequestManager::nickname;
int RequestManager::maxCategories = 5;
double RequestManager::ipLatitude = 0, RequestManager::ipLongitude = 0;

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
}

void RequestManager::init(bool geoSourceUnavailable)
{
    _requestIpGeo = geoSourceUnavailable;
    updateUserInfo();

    auto timer = new QTimer;
    connect(timer, SIGNAL(timeout()), SLOT(updateUserInfo()));
    timer->start(UpdateUserInfoMsec);
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
    connect(reply, &QNetworkReply::finished, [reply, callback, this]{
        TextItemList result;
        if (reply->error() == QNetworkReply::NoError)
            for (const auto &value : arrayFromReply(reply))
                result += feedItemFromJson(value.toObject());
        callback(result);
    });
}

void RequestManager::requestComments(quint32 postId, FeedLambda callback, quint32 commentIdForOlderPosts)
{
    _qnam->post(requestFromUrlParts(QLatin1String("Horn/Entry/"), false), QString("{\"horn_id\": %1}").arg(postId).toLatin1());

    QString json;
    if (commentIdForOlderPosts)
    {
        QJsonObject obj({{"limit", kCommentsPerPage}});
        obj["conditions"] = QJsonObject({{ "5", QJsonObject({{"id", QString::number(commentIdForOlderPosts)}}) }});
        json = dataFromJsonObj(obj);
    }

    auto reply = _qnam->get(requestFromUrlParts(QString("Horn/%1/Comment/").arg(postId), true, json));
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
        callback(reply->error() == QNetworkReply::NoError && !response.isEmpty());
    });
}

void RequestManager::createPost(const QString &message, const QStringList &tags, double latitude, double longitude, quint32 imageId, SuccessLambda callback)
{
    QJsonObject dic;
    dic["message"] = message;
    dic["lang"] = "ru";
    dic["hashtags"] = QJsonArray::fromStringList(tags);

    if (!qIsNaN(latitude) && !qIsNaN(longitude))
    {
        QJsonObject geo;
        geo["type"] = "Point";
        geo["coordinates"] = QJsonArray({longitude, latitude});
        dic["geometry"] = geo;
    }

    if (imageId)
        dic["image"] = QJsonObject({{"id", QString::number(imageId)}, {"itype", "2"}});
    else
        dic["bg"] = "white:sample";

    auto reply = _qnam->post(requestFromUrlParts("Horn/", false), dataFromJsonObj(dic));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        qDebug() << reply->readAll(); // "{"Errors":{"ERROR_USER_CANT_POST":[]}}"
        callback(reply->error() == QNetworkReply::NoError);
    });
}

void RequestManager::uploadImage(QIODevice *device, std::function<void(const QJsonObject &)> callback)
{
    auto request = QNetworkRequest(QUrl(QString("%1?token=%2").arg(HornUploadImageBaseUrl, token)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    auto reply = _qnam->post(request, device);
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        callback(reply->error() == QNetworkReply::NoError ?  QJsonDocument::fromJson(reply->readAll()).object() : QJsonObject());
    });
}

void RequestManager::requestNotifications(FeedLambda callback)
{
    QJsonObject dic({ {"conditions", QJsonObject({ {"0", QJsonObject({ {"lang", "ru"} })} })} });
    auto reply = _qnam->get(requestFromUrlParts(QLatin1String("UserNotification/"), true, dataFromJsonObj(dic)));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        TextItemList result;
        if (reply->error() == QNetworkReply::NoError)
        {
            for (const auto &value : arrayFromReply(reply))
            {
                auto item = new NotificationItem;
                auto dic = value.toObject();
                item->setupFromJson(dic);
                item->isRead = dic["read"].toString() != "0";
                item->postId = dic["horn_id"].toDouble();
                item->commentId1 = dic["cid"].toDouble();
                item->commentId2 = dic["pcid"].toDouble();
                item->type = dic["type"].toString();
                result += item;
            }
        }
        callback(result);
    });
}

void RequestManager::requestPostWithId(quint32 postId, std::function<void(FeedItem *)> callback)
{
    auto reply = _qnam->get(requestFromUrlParts(QString("Horn/%1").arg(postId)));
    connect(reply, &QNetworkReply::finished, [reply, callback, this]{
        FeedItem *item = 0;
        if (reply->error() == QNetworkReply::NoError)
            item = feedItemFromJson(QJsonDocument::fromJson(reply->readAll()).object());
        callback(item);
    });
}

void RequestManager::markNotificationsRead(const QList<quint32> &ids)
{
    if (!ids.isEmpty())
        patchRequest("UserNotification/" + arrayRequestParam(ids), "{\"read\": 1}");
}

void RequestManager::requestCommentsVotes(const QList<quint32> &ids, std::function<void(const QHash<decltype(CommentItem::id), bool> &)> callback)
{
    if (ids.isEmpty())
        return;

    auto reply = _qnam->get(requestFromUrlParts("HornCommentVote/" + arrayRequestParam(ids)));
    connect(reply, &QNetworkReply::finished, [reply, callback, this]{
        QHash<decltype(CommentItem::id), bool> votesHash;
        if (reply->error() == QNetworkReply::NoError)
        {
            for (const auto &value : QJsonDocument::fromJson(reply->readAll()).object()["votes"].toArray())
            {
                auto dic = value.toObject();
                votesHash[dic["id"].toString().toUInt()] = dic["vote"] == "1";
            }
        }
        callback(votesHash);
    });
}

void RequestManager::changeCommentVote(quint32 commentId, bool deleteVote, bool upvote, SuccessLambda callback)
{
    changeVote(QString("Horn/Comment/%1/Vote/").arg(commentId), deleteVote, upvote, callback);
}

void RequestManager::changePostVote(quint32 postId, bool deleteVote, bool upvote, SuccessLambda callback)
{
    changeVote(QString("Horn/%1/Vote/").arg(postId), deleteVote, upvote, callback);
}

// private

void RequestManager::requestAuth()
{
    patchRequest(QString("Auth/%1").arg(token), "{}");
}

void RequestManager::requestUserInfo()
{
    auto reply = _qnam->get(requestFromUrlParts(QString("User/%1").arg(userHashIdentifier)));
    connect(reply, &QNetworkReply::finished, [reply, this]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto dic = QJsonDocument::fromJson(reply->readAll()).object();
            qDebug() << "user info:" << dic;

            maxCategories = dic["settings"].toObject()["max_hashtags"].toString().toInt();

            nickname = dic["nickname"].toString();
            emit nicknameChanged(nickname);
        }
    });
}

void RequestManager::requestCategories()
{
    auto reply = _qnam->get(requestFromUrlParts(QLatin1String("DictCategory/"), true, QLatin1String("{\"conditions\":{\"0\":{\"lang\":\"ru\"}}}")));
    connect(reply, &QNetworkReply::finished, [reply, this]{
        if (reply->error() == QNetworkReply::NoError)
        {
            for (const auto &value : arrayFromReply(reply))
            {
                auto dic = value.toObject();
                _categoriesMap.insert(dic["id"].toString(), dic["descr"].toString());
            }
        }
    });
}

void RequestManager::requestGeoInfo()
{
    auto reply = _qnam->get(requestFromUrlParts(QLatin1String("IpGeo/1")));
    connect(reply, &QNetworkReply::finished, [reply, this]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto dic = QJsonDocument::fromJson(reply->readAll()).object();
            qDebug() << "IpGeo:" << dic;

            auto coordinates = dic["geometry"].toObject()["coordinates"].toArray();
            if (coordinates.size() == 2)
            {
                ipLatitude  = coordinates.at(1).toDouble();
                ipLongitude = coordinates.at(0).toDouble();
            }
        }
    });
}

void RequestManager::changeVote(const QString &urlPart, bool deleteVote, bool upvote, SuccessLambda callback)
{
    QNetworkReply *reply;
    if (deleteVote)
        reply = _qnam->deleteResource(requestFromUrlParts(urlPart + QLatin1String("1")));
    else
        reply = _qnam->post(requestFromUrlParts(urlPart, false), dataFromJsonObj({{"vote", upvote ? 1 : -1}}));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        callback(reply->error() == QNetworkReply::NoError);
    });
}

void RequestManager::patchRequest(const QString &urlPart, const QByteArray &data, SuccessLambda callback)
{
    auto buf = new QBuffer;
    buf->buffer() = data;
    auto deleteBufLambda = [buf]{ delete buf; };

    if (buf->open(QIODevice::ReadOnly))
    {
        auto reply = _qnam->sendCustomRequest(requestFromUrlParts(urlPart), "PATCH", buf);
        connect(reply, &QNetworkReply::finished, [reply, callback, deleteBufLambda]{
            callback(reply->error() == QNetworkReply::NoError);
            deleteBufLambda();
        });
    }
    else
        deleteBufLambda();
}

QNetworkRequest RequestManager::requestFromUrlParts(const QString &urlPart, bool get, const QString &urlJsonText)
{
    auto urlString = kHornAppBaseUrl + urlPart + QLatin1String("?token=") + token;
    if (!urlJsonText.isEmpty())
        urlString += QLatin1String("&json=") + QString::fromUtf8(QUrl::toPercentEncoding(urlJsonText));

    QNetworkRequest request{QUrl(urlString)};
    if (!get)
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json; charset=utf-8"));
    return request;
}

FeedItem *RequestManager::feedItemFromJson(const QJsonObject &jsonObj)
{
    auto item = new FeedItem;
    item->setupFromJson(jsonObj);

    item->comments = jsonObj["scomms"].toString().toInt();

    QStringList tags;
    for (const auto &tag : jsonObj["hashtags"].toArray())
        tags << categoryNameFromId(tag.toString());
    item->tags = tags;

    auto background = jsonObj["bg"].toString();
    if (background.isEmpty())
        background = jsonObj["image"].toObject()["link"].toString();
    item->background = background;

    auto coordinates = jsonObj["centroid"].toObject()["coordinates"].toArray();
    if (coordinates.size() == 2)
    {
        auto latitude = coordinates.at(1).toDouble(), longitude = coordinates.at(0).toDouble();
        if (latitude || longitude) // both == 0 => missing
            item->coordinates = QGeoCoordinate(latitude, longitude);
    }

    return item;
}

QJsonArray RequestManager::arrayFromReply(QNetworkReply *reply)
{
    return QJsonDocument::fromJson(reply->readAll()).array();
}

QByteArray RequestManager::dataFromJsonObj(const QJsonObject &jsonObj)
{
    return QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
}

QString RequestManager::arrayRequestParam(const QList<quint32> &ids)
{
    QStringList stringIds;
    for (auto id : ids)
        stringIds << QString::number(id);
    return "%5B" + stringIds.join(',') + "%5D"; // [id1, id2, ..., idN]
}
