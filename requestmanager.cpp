#include "requestmanager.h"

#include <QJsonDocument>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

static const QLatin1String kHornAppBaseUrl("http://app.hornapp.com/request/v1/");
static const QLatin1String kToken("d7f346b2a426f2d9b18c255c605e9d27053a247adaafcb56d9ef00f5c20f240b"), kUserId("525c87a74549fa59bd41829815f024c21cc352fe6ba85aa047a3e1c73f53cf2f");

enum
{
    Auth,
    User,
    Geo,
    New
};

RequestManager::RequestManager(QObject *parent) : QObject(parent), _qnam(new QNetworkAccessManager)
{
//    QNetworkRequest request(QUrl(kHornAppBaseUrl + ));
//    request.setHeader(QNetworkRequest::UserAgentHeader, "HornAppDesktop/0.1 (Windows 8.1 x64)");
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
//    request.setHeader(QNetworkRequest::ContentLengthHeader, 2);
//    request.setRawHeader("Accept-Language", "ru;q=1, en;q=0.9");
//    request.setRawHeader("Accept", "*/*");
//    request.setRawHeader("Connection", "keep-alive");
//    request.setRawHeader("Connection", "keep-alive");
//    request.setRawHeader("Host", "app.hornapp.com");
//    request.setRawHeader("Accept-Encoding", "gzip, deflate");

    connect(_qnam, &QNetworkAccessManager::finished, [](QNetworkReply *reply) {
        bool ok = reply->error() == QNetworkReply::NoError;
        qDebug() << reply->rawHeaderPairs();
        QByteArray data = reply->readAll();
        qDebug() << data;

        if (ok)
        {
            switch (reply->property("type").toInt()) {
            case User:
            {
                QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
            }
                break;
            default:
                break;
            }
        }
        reply->deleteLater();
    });

    _patchRequestDataBuffer.setData("{}", 2);
    _patchRequestDataBuffer.open(QIODevice::ReadOnly);

    /*QNetworkReply *reply =*/ _qnam->sendCustomRequest(requestFromUrlPart(QString("Auth/%1").arg(kToken)), "PATCH", &_patchRequestDataBuffer)->setProperty("type", Auth);
    _qnam->get(requestFromUrlPart(QString("User/%1").arg(kUserId)))->setProperty("type", User);
    _qnam->get(requestFromUrlPart(QLatin1String("IpGeo/1")))->setProperty("type", Geo);
    _qnam->get(requestFromUrlPart(QLatin1String("Horn/New/"), QLatin1String("{\"limit\":50,\"conditions\":{\"0\":{\"lang\":\"ru\"}}}")))->setProperty("type", New);
//    connect(reply, &QNetworkReply::finished, [reply, this] {
//        qDebug("auth finished");
//        bool ok = reply->error() == QNetworkReply::NoError;
//        reply->deleteLater();

//        if (ok)
//        {
//            QNetworkReply *reply = _qnam->get(requestFromUrlPart(QString("User/%1").arg(kUserId)));
//            connect(reply, &QNetworkReply::finished, [reply] {
//                qDebug("login finished");
//                QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
//                reply->deleteLater();
//                qDebug() << json;
//            });
//        }
//    });
//    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [reply](QNetworkReply::NetworkError code) {
//        qDebug() << "error" << code << reply->errorString();
//        qDebug() << reply->rawHeaderPairs();
//        reply->deleteLater();
//    });
}

QNetworkRequest RequestManager::requestFromUrlPart(const QString &urlPart, const QString &urlJsonText)
{
    QString urlString = kHornAppBaseUrl + QString("%1?token=%2").arg(urlPart, kToken);
    if (!urlJsonText.isEmpty())
        urlString += QLatin1String("&json=") + QString::fromUtf8(QUrl::toPercentEncoding(urlJsonText));

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setOriginatingObject(this);
    if (!urlJsonText.isEmpty())
    {
        qDebug() << urlString;
        qDebug() << request.url();
    }
    request.setHeader(QNetworkRequest::UserAgentHeader, "HornAppDesktop/0.1 (Windows 8.1 x64)");
    return request;
}
