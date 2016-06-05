#include "feedimagecache.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFileInfo>

#include <QImage>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#define WRITABLE_CACHE_PATH QStandardPaths::writableLocation(QStandardPaths::CacheLocation)

QCache<QString, QImage> FeedImageCache::_imageCache;
QNetworkAccessManager   *FeedImageCache::_qnam;

void FeedImageCache::getImageFromUrl(const QString &urlString, std::function<void(QImage *)> successCallback)
{
    if (!urlString.startsWith("http"))
        return;

    auto image = _imageCache[urlString];
    if (!image)
    {
        auto cachedImagePath = QStandardPaths::locate(QStandardPaths::CacheLocation, QFileInfo(urlString).fileName());
        if (!cachedImagePath.isEmpty())
        {
            QImage cachedImage(cachedImagePath);
            if (!cachedImage.isNull())
            {
                image = new QImage(cachedImage);
                _imageCache.insert(urlString, image);
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
            _imageCache.insert(urlString, image);

            QDir().mkpath(WRITABLE_CACHE_PATH);
            image->save(savePathForUrl(urlString));
        }
    });
}

void FeedImageCache::cleanCache()
{
    QDateTime yesterday(QDate::currentDate().addDays(-1), QTime::currentTime());
    for (const auto &fi : QDir(WRITABLE_CACHE_PATH, QString(), QDir::Time, QDir::Files | QDir::NoDotAndDotDot).entryInfoList())
        if (fi.created() <= yesterday)
            QFile::remove(fi.filePath());
}

void FeedImageCache::copyFileToCache(const QString &fileName, const QString &urlString)
{
    QFile::copy(fileName, savePathForUrl(urlString));
}

QString FeedImageCache::savePathForUrl(const QString &urlString)
{
    return WRITABLE_CACHE_PATH + "/" + QFileInfo(urlString).fileName();
}
