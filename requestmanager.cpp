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
static const QLatin1String kToken("d7f346b2a426f2d9b18c255c605e9d27053a247adaafcb56d9ef00f5c20f240b"), kUserId("525c87a74549fa59bd41829815f024c21cc352fe6ba85aa047a3e1c73f53cf2f");

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

void RequestManager::requestNewPosts(FeedLambda callback)
{
    auto reply = _qnam->get(requestFromUrlParts(QLatin1String("Horn/New/"), true, QLatin1String("{\"limit\":50,\"conditions\":{\"0\":{\"lang\":\"ru\"}}}")));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
        if (reply->error() == QNetworkReply::NoError)
        {
            TextItemList result;
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
            callback(result);
        }
    });
}

void RequestManager::requestComments(quint32 postId, FeedLambda callback)
{
    QJsonObject dic;
    dic["horn_id"] = QJsonValue(static_cast<int>(postId));

    _qnam->post(requestFromUrlParts(QLatin1String("Horn/Entry/"), false), QJsonDocument(dic).toJson(QJsonDocument::Compact));

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

                result += item;
            }
            callback(result);
        }
    });
}

void RequestManager::postComment(quint32 postId, const QString &comment, std::function<void(bool)> callback)
{
    // TODO: add support for parent_id
    QJsonObject dic;
    dic["message"] = QJsonValue(comment);

    auto reply = _qnam->post(requestFromUrlParts(QString("Horn/%1/Comment/").arg(postId), false), QJsonDocument(dic).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, [reply, callback]{
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
    auto reply = _qnam->get(requestFromUrlParts(QString("User/%1").arg(kUserId)));
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
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2 (Windows 8.1 x64)").arg(qApp->applicationName(), qApp->applicationVersion()));
    if (!get)
        request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json; charset=utf-8"));
    return request;
}

QJsonArray RequestManager::arrayFromReply(QNetworkReply *reply)
{
    return QJsonDocument::fromJson(reply->readAll()).array();
}
