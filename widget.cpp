#include "ui_widget.h"
#include    "widget.h"
#include "requestmanager.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    RequestManager::instance().sendNewPostsRequest();
}

Widget::~Widget()
{
    delete ui;
}
