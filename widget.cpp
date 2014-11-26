#include "ui_widget.h"
#include "widget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"

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
        RequestManager::instance().sendCommentsRequest(item->id, [item, this](const QList<TextItem *> &comments) {
            qDebug() << "post" << item->id << "has" << comments.size() << "comments";
            for (const auto &commentItem : comments)
            {
                CommentItem *comment = static_cast<CommentItem *>(commentItem);
                qDebug() << comment->reputation << comment->nickname << ":" << comment->message;
            }
        });
    });

    RequestManager::instance().sendNewPostsRequest([this](const QList<TextItem *> &feed) {
        _feedModel->setDataSource(feed);
    });
}

Widget::~Widget()
{
    delete ui;
}
