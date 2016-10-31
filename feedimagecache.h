#ifndef FEEDIMAGECACHE_H
#define FEEDIMAGECACHE_H

#include <QString>
#include <QCache>

#include <functional>

class QNetworkAccessManager;
class QImage;
struct FeedItem;

class FeedImageCache
{
public:
    static void getImageForItem(FeedItem *item, std::function<void(QImage *)> successCallback);
    static void cleanCache(); // remove images older than 24h
    static void copyFileToCache(const QString &fileName, const QString &itemIdString);
    static QString savePathForItem(FeedItem *item);

private:
    static QCache<QString, QImage> _imageCache;
    static QNetworkAccessManager *_qnam;

    static QString savePathForId(const QString &itemIdString);
};

#endif // FEEDIMAGECACHE_H
