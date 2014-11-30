#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"

FeedItemWidget::FeedItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FeedItemWidget)
{
    ui->setupUi(this);
}

FeedItemWidget::~FeedItemWidget()
{
    delete ui;
}
