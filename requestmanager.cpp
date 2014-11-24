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
//        qDebug() << reply->rawHeaderPairs();
//        qDebug() << reply->readAll();
        reply->deleteLater();
    });

    sendAuthRequest();
    sendUserRequest();
    sendGeoRequest();
}

void RequestManager::sendNewPostsRequest()
{
    auto reply = _qnam->get(requestFromUrlParts(QLatin1String("Horn/New/"), QLatin1String("{\"limit\":20,\"conditions\":{\"0\":{\"lang\":\"ru\"}}}")));
    connect(reply, &QNetworkReply::finished, [reply]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto array = QJsonDocument::fromJson(reply->readAll()).array();
            for (const auto &value : array)
            {
                auto dic = value.toObject();
                auto message = dic["message"].toString();
                auto id = dic["id"].toString().toInt();
                auto comments = dic["scomms"].toString().toInt();
                auto votes = dic["svotes"].toString().toInt();
                qDebug() << id << votes << comments << message;
            }
        }
    });
}

// private

void RequestManager::sendAuthRequest()
{
    auto buf = new QBuffer;
    buf->buffer() = "{}";
    auto deleteBufLambda = [buf]{ delete buf; };

    if (buf->open(QIODevice::ReadOnly))
        connect(_qnam->sendCustomRequest(requestFromUrlParts(QString("Auth/%1").arg(kToken)), "PATCH", buf), &QNetworkReply::finished, deleteBufLambda);
    else
        deleteBufLambda();
}

void RequestManager::sendUserRequest()
{
    auto reply = _qnam->get(requestFromUrlParts(QString("User/%1").arg(kUserId)));
    connect(reply, &QNetworkReply::finished, [reply]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto dic = QJsonDocument::fromJson(reply->readAll()).object();
            auto nickname = dic["nickname"].toString();
            auto reputation = dic["rep"].toString().toInt();
            qDebug() << nickname << "=" << reputation;
        }
    });
}

void RequestManager::sendGeoRequest()
{
    _qnam->get(requestFromUrlParts(QLatin1String("IpGeo/1")));
}

QNetworkRequest RequestManager::requestFromUrlParts(const QString &urlPart, const QString &urlJsonText)
{
    auto urlString = kHornAppBaseUrl + QString("%1?token=%2").arg(urlPart, kToken);
    if (!urlJsonText.isEmpty())
        urlString += QLatin1String("&json=") + QString::fromUtf8(QUrl::toPercentEncoding(urlJsonText));

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2 (Windows 8.1 x64)").arg(qApp->applicationName(), qApp->applicationVersion()));
    return request;
}
