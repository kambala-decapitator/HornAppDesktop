#ifndef FEEDLISTVIEW_HPP
#define FEEDLISTVIEW_HPP

#include <QListView>
#include <QMouseEvent>

class FeedListView : public QListView
{
public:
    explicit FeedListView(QWidget *parent = 0) : QListView(parent) {}

protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (o->inherits("QLabel") && e->type() == QEvent::MouseButtonDblClick)
        {
            auto me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::LeftButton)
            {
                emit doubleClicked(indexAt(me->pos()));
                return true;
            }
        }
        return QListView::eventFilter(o, e);
    }
};

#endif // FEEDLISTVIEW_HPP
