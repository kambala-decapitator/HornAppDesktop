#ifndef FEEDIMAGECACHE_H
#define FEEDIMAGECACHE_H

#include <QString>
#include <QHash>

#include <functional>

class QNetworkAccessManager;
class QImage;

class FeedImageCache
{
public:
    static void getImageFromUrl(const QString &urlString, std::function<void(QImage *)> successCallback);

private:
    static QHash<QString, QImage *> _imageHash;
    static QNetworkAccessManager *_qnam;
};

#endif // FEEDIMAGECACHE_H
