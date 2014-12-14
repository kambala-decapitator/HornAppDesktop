#include "commentswidget.h"
#include "ui_commentswidget.h"

CommentsWidget::CommentsWidget(FeedItem *feedItem, const TextItemList &comments, QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f),
    ui(new Ui::CommentsWidget), _comments(comments), _recipientCommentId(0)
{
    ui->setupUi(this);

    ui->messageLabel->setText(QString("%1\n%2 | %3").arg(feedItem->message).arg(feedItem->comments).arg(feedItem->reputation));

    for (const auto &item : comments)
    {
        CommentItem *comment = static_cast<CommentItem *>(item);
        addComment(comment->message, comment->nickname, comment->reputation, comment->recipientNickname);
    }

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item){
        int i = ui->listWidget->row(item);
        if (i >= _comments.size())
            return;

        CommentItem *comment = static_cast<CommentItem *>(_comments.at(i));
        _recipientCommentId = comment->id;

        QString oldNickname = _recipientNickname;
        if (!oldNickname.isEmpty())
            oldNickname = appealTo(oldNickname);
        _recipientNickname = comment->nickname;

        QString commentText = ui->plainTextEdit->toPlainText();
        QTextCursor currentTextCursor = ui->plainTextEdit->textCursor();
        if (!oldNickname.isEmpty() && commentText.indexOf(oldNickname) != -1)
        {
            commentText.replace(0, oldNickname.length(), appealTo(_recipientNickname));
            ui->plainTextEdit->setPlainText(commentText);
        }
        else
        {
            ui->plainTextEdit->setTextCursor(QTextCursor(ui->plainTextEdit->document()));
            ui->plainTextEdit->insertPlainText(appealTo(_recipientNickname));
        }
        ui->plainTextEdit->setTextCursor(currentTextCursor);

        ui->plainTextEdit->setFocus();
    });

    connect(ui->sendButton, &QPushButton::clicked, [feedItem, this]{
        QString comment = ui->plainTextEdit->toPlainText(), commentToSend = comment;
        RequestManager::instance().postComment(feedItem->id, comment.remove(appealTo(_recipientNickname)), _recipientCommentId, [comment, this](bool ok){
            if (ok)
            {
                addComment(comment);
                ui->plainTextEdit->clear();

                _recipientCommentId = 0;
                _recipientNickname.clear();
                // TODO: update comments counter in current widget and in parent feed
            }
        });
    });
}

CommentsWidget::~CommentsWidget()
{
    delete ui;
    qDeleteAll(_comments);
}

void CommentsWidget::addComment(const QString &comment, const QString &nickname, qint32 reputation, const QString &recipient)
{
    QString text = QString("%1 %2: ").arg(reputation).arg(nickname);
    if (!recipient.isEmpty())
        text += appealTo(recipient);
    ui->listWidget->addItem(text + comment);
}

QString CommentsWidget::appealTo(const QString &recipient) const
{
    return recipient + ", ";
}
