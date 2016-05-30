#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

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

    void requestFeed();
    void loadNextPosts();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void showEvent(QShowEvent *);

private:
    Ui::FeedWidget *ui;
    FeedListModel *_feedModel;
    QString _requestPart;
    bool _requestFeedOnFirstShow;

#ifdef Q_OS_MAC
    void showLocation(double latitude, double longitude);
#endif
};

#endif // WIDGET_H
