#ifndef FEEDITEMWIDGET_H
#define FEEDITEMWIDGET_H

#include <QWidget>

namespace Ui {
class FeedItemWidget;
}

class FeedItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeedItemWidget(QWidget *parent = 0);
    virtual ~FeedItemWidget();

public:
    Ui::FeedItemWidget *ui;
};

#endif // FEEDITEMWIDGET_H
