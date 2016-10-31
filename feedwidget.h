#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <functional>

#include "feeditem.h"

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
    void showEvent(QShowEvent *);

private slots:
    void loadNextPosts();

private:
    Ui::FeedWidget *ui;
    FeedListModel *_feedModel;
    QString _requestPart;
    bool _requestFeedOnFirstShow;

    void requestFeed(decltype(FeedItem::id) postIdForOlderFeed, std::function<int(const TextItemList &)> processFeedCallback, std::function<void(void)> afterDisplayingFeedCallback = []{});
#ifdef Q_OS_MACOS
    void quickLookImage(const QString &imagePath);
    void showLocation(double latitude, double longitude);
#endif
};

#endif // WIDGET_H
