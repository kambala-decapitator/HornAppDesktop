#include "commentswidget.h"
#include "ui_commentswidget.h"

CommentsWidget::CommentsWidget(FeedItem *feedItem, const TextItemList &comments, QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f), ui(new Ui::CommentsWidget)
{
    ui->setupUi(this);

    ui->messageLabel->setText(QString("%1\n%2 | %3").arg(feedItem->message).arg(feedItem->comments).arg(feedItem->reputation));

    for (const auto &item : comments)
    {
        CommentItem *comment = static_cast<CommentItem *>(item);
        addComment(comment->message, comment->nickname, comment->reputation, comment->recipientNickname);
    }

    connect(ui->sendButton, &QPushButton::clicked, [feedItem, this]{
        QString comment = ui->plainTextEdit->toPlainText();
        RequestManager::instance().postComment(feedItem->id, comment, [comment, this](bool ok){
            if (ok)
            {
                addComment(comment);
                ui->plainTextEdit->clear();
                // TODO: update comments counter in current widget and in parent feed
            }
        });
    });
}

CommentsWidget::~CommentsWidget()
{
    delete ui;
}

void CommentsWidget::addComment(const QString &comment, const QString &nickname, qint32 reputation, const QString &recipient)
{
    QString text = QString("%1 %2: ").arg(reputation).arg(nickname);
    if (!recipient.isEmpty())
        text += recipient + ", ";
    ui->listWidget->addItem(text + comment);
}
