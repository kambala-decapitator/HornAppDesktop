#ifndef FEEDIMAGECACHE_H
#define FEEDIMAGECACHE_H

#include <QString>
#include <QCache>

#include <functional>

class QNetworkAccessManager;
class QImage;

class FeedImageCache
{
public:
    static void getImageFromUrl(const QString &urlString, std::function<void(QImage *)> successCallback);

private:
    static QCache<QString, QImage> _imageCache;
    static QNetworkAccessManager *_qnam;
};

#endif // FEEDIMAGECACHE_H
