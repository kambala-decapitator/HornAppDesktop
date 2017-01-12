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
                // not using simple me->pos() because it returns not the clicked point resulting in index being invalid when label contains a word wider than listview
                // for an example see post 163452
                emit doubleClicked(indexAt(mapFromGlobal(me->globalPos())));
                return true;
            }
        }
        return QListView::eventFilter(o, e);
    }
};

#endif // FEEDLISTVIEW_HPP
