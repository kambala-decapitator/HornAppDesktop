#include "ui_widget.h"
#include "widget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"
#include "commentswidget.h"

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget), _feedModel(new FeedListModel(this))
{
    ui->setupUi(this);

    ui->listView->setModel(_feedModel);
    ui->listView->setItemDelegate(new FeedItemDelegate(this));

    connect(ui->listView, &QListView::doubleClicked, [this](const QModelIndex &index) {
        FeedItem *item = _feedModel->itemAtModelIndex(index);
        RequestManager::instance().sendCommentsRequest(item->id, [item, this](const TextItemList &comments) {
            CommentsWidget *w = new CommentsWidget(item, comments, this, Qt::Window);
            w->show();
        });
    });

    RequestManager::instance().sendNewPostsRequest([this](const TextItemList &feed) {
        _feedModel->setDataSource(feed);
        for (int i = 0; i < feed.size(); ++i)
            ui->listView->openPersistentEditor(_feedModel->index(i));
    });
}

Widget::~Widget()
{
    delete ui;
}
