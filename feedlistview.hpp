#ifndef FEEDLISTVIEW_HPP
#define FEEDLISTVIEW_HPP

#include <QListView>
#include <QMouseEvent>
#include <QApplication>

class FeedListView : public QListView
{
public:
    explicit FeedListView(QWidget *parent = 0) : QListView(parent) {}
    virtual ~FeedListView() {}

protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (o->inherits("QLabel") && e->type() == QEvent::MouseButtonDblClick && static_cast<QMouseEvent *>(e)->button() == Qt::LeftButton)
        {
            qApp->sendEvent(this, e);
            return true;
        }
        return QListView::eventFilter(o, e);
    }
};

#endif // FEEDLISTVIEW_HPP
