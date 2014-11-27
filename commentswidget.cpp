#include "commentswidget.h"
#include "ui_commentswidget.h"

CommentsWidget::CommentsWidget(FeedItem *feedItem, const TextItemList &comments, QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f), ui(new Ui::CommentsWidget)
{
    ui->setupUi(this);

    ui->messageLabel->setText(QString("%1\n%2 | %3").arg(feedItem->message).arg(feedItem->comments).arg(feedItem->reputation));

    for (const auto &item : comments)
    {
        CommentItem *comment = static_cast<CommentItem *>(item);
        ui->listWidget->addItem(QString("%1 %2: %3").arg(comment->reputation).arg(comment->nickname, comment->message));
    }
}

CommentsWidget::~CommentsWidget()
{
    delete ui;
}
