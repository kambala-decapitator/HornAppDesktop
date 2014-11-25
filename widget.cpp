#include "ui_widget.h"
#include "widget.h"
#include "requestmanager.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    RequestManager::instance().sendNewPostsRequest([this](const QList<FeedItem> &feed) {
        qDebug("got %d items", feed.size());
    });
}

Widget::~Widget()
{
    delete ui;
}
