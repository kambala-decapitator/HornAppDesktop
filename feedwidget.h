#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class FeedWidget;
}

class FeedListModel;

class FeedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeedWidget(const QString &requestPart, QWidget *parent = 0);
    ~FeedWidget();

    void requestFeed();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void showEvent(QShowEvent *);

private:
    Ui::FeedWidget *ui;
    FeedListModel *_feedModel;
    QString _requestPart;
    bool _requestFeedOnFirstShow;
};

#endif // WIDGET_H
