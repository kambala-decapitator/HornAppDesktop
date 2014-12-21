#include "feedimagecache.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

#include <QImage>

#include <QNetworkAccessManager>
#include <QNetworkReply>

QHash<QString, QImage *> FeedImageCache::_imageHash;
QNetworkAccessManager   *FeedImageCache::_qnam;

void FeedImageCache::getImageFromUrl(const QString &urlString, std::function<void(QImage *)> successCallback)
{
    if (!urlString.startsWith("http"))
        return;

    auto image = _imageHash.value(urlString);
    if (!image)
    {
        auto cachedImagePath = QStandardPaths::locate(QStandardPaths::CacheLocation, QFileInfo(urlString).fileName());
        if (!cachedImagePath.isEmpty())
        {
            QImage cachedImage(cachedImagePath);
            if (!cachedImage.isNull())
            {
                image = new QImage(cachedImage);
                _imageHash.insert(urlString, image);
            }
        }
    }
    if (image)
    {
        successCallback(image);
        return;
    }

    if (!_qnam)
        _qnam = new QNetworkAccessManager;
    auto reply = _qnam->get(QNetworkRequest(QUrl(urlString)));
    QObject::connect(reply, &QNetworkReply::finished, [reply, urlString, successCallback]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto image = new QImage;
            image->loadFromData(reply->readAll());

            successCallback(image);
            _imageHash.insert(urlString, image);

            auto cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
            QDir().mkpath(cachePath);
            image->save(cachePath + "/" + QFileInfo(urlString).fileName());
        }
    });
}
