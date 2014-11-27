#ifndef COMMENTSWIDGET_H
#define COMMENTSWIDGET_H

#include <QWidget>

#include "feeditem.h"

namespace Ui {
class CommentsWidget;
}

class CommentsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommentsWidget(FeedItem *feedItem, const TextItemList &comments, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~CommentsWidget();

private:
    Ui::CommentsWidget *ui;
};

#endif // COMMENTSWIDGET_H
