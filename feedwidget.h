#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QModelIndex>
#include <functional>
#include "feeditem.h"

#ifndef Q_OS_MACOS
#include <QPointer>
#include <QLabel>
#define CUSTOM_IMAGE_WINDOW
#endif

namespace Ui {
class FeedWidget;
}
class FeedListModel;
class QGeoPositionInfoSource;

class FeedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeedWidget(const QString &requestPart, QGeoPositionInfoSource *geoSource, QWidget *parent = 0);
    ~FeedWidget();

public slots:
    void requestFeed();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void loadNextPosts();
    void openImage();

private:
    Ui::FeedWidget *ui;
    FeedListModel *_feedModel;
    QString _requestPart;
    bool _requestFeedOnFirstShow = true;
    QModelIndex _openImageModelIndex = QModelIndex();
#ifdef CUSTOM_IMAGE_WINDOW
    QPointer<QLabel> _imageWindow = nullptr;
#endif

    void requestFeed(decltype(FeedItem::id) postIdForOlderFeed, std::function<int(const TextItemList &)> processFeedCallback, std::function<void(void)> afterDisplayingFeedCallback = []{});
#ifdef Q_OS_MACOS
    void quickLookImage(const QString &imagePath, const QRect &rect);
    void showLocation(double latitude, double longitude);
#endif
};

#endif // WIDGET_H
