#include "ui_widget.h"
#include "widget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget), _feedModel(new FeedListModel(this))
{
    ui->setupUi(this);

    ui->listView->setModel(_feedModel);
    ui->listView->setItemDelegate(new FeedItemDelegate(this));

    RequestManager::instance().sendNewPostsRequest([this](const QList<FeedItem> &feed) {
        _feedModel->setDataSource(feed);
    });
}

Widget::~Widget()
{
    delete ui;
}
