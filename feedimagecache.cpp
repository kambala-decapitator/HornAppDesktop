#include "feedimagecache.h"
#include "feeditem.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFileInfo>

#include <QImage>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#define WRITABLE_CACHE_PATH QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
#define FILENAME_FOR_ID(ID) (ID + QLatin1String(".png"))

QCache<QString, QImage> FeedImageCache::_imageCache;
QNetworkAccessManager   *FeedImageCache::_qnam;

void FeedImageCache::getImageForItem(FeedItem *item, std::function<void(QImage *)> successCallback)
{
    if (!item)
        return;
    auto urlString = item->background;
    if (!urlString.startsWith("http"))
        return;

    auto itemIdString = QString::number(item->id);
    auto image = _imageCache[urlString];
    if (!image)
    {
        auto cachedImagePath = QStandardPaths::locate(QStandardPaths::CacheLocation, FILENAME_FOR_ID(itemIdString));
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
    QObject::connect(reply, &QNetworkReply::finished, [itemIdString, reply, urlString, successCallback]{
        if (reply->error() == QNetworkReply::NoError)
        {
            auto image = new QImage;
            image->loadFromData(reply->readAll());

            successCallback(image);
            _imageCache.insert(urlString, image);

            QDir().mkpath(WRITABLE_CACHE_PATH);
            image->save(savePathForId(itemIdString));
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

void FeedImageCache::copyFileToCache(const QString &fileName, const QString &itemIdString)
{
    QFile::copy(fileName, savePathForId(itemIdString));
}

QString FeedImageCache::savePathForId(const QString &itemIdString)
{
    return WRITABLE_CACHE_PATH + "/" + FILENAME_FOR_ID(itemIdString);
}
