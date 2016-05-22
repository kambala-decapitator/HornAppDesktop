#include "commentswidget.h"
#include "ui_commentswidget.h"

CommentsWidget::CommentsWidget(FeedItem *feedItem, const TextItemList &comments, bool deleteItem, const QSet<quint32> &highlightedComments, QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f),
    ui(new Ui::CommentsWidget), _comments(comments), _recipientCommentId(0), _feedItem(feedItem), _deleteItem(deleteItem)
{
    ui->setupUi(this);

    ui->messageLabel->setText(QString("%1\n%2 | %3").arg(feedItem->message).arg(feedItem->comments).arg(feedItem->reputation));
    showComments(highlightedComments);

    installEventFilter(this);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item){
        int i = ui->listWidget->row(item);
        if (i >= _comments.size())
            return;

        CommentItem *comment = static_cast<CommentItem *>(_comments.at(i));
        if (comment->nickname == RequestManager::instance().userNickname())
            return;
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

    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, [this]{
        ui->charactersCountLabel->setText(QString::number(ui->plainTextEdit->toPlainText().size()));
    });

    connect(ui->reloadButton, &QPushButton::clicked, [this]{
        // GET /request/v1/Horn/postID?token=...
        RequestManager::instance().requestComments(_feedItem->id, [this](const TextItemList &newComments) {
            if (!newComments.isEmpty())
            {
                qDeleteAll(_comments);
                _comments = newComments;

                ui->listWidget->clear();
                showComments();
                ui->listWidget->scrollToBottom();
            }
        });
    });

    connect(ui->sendButton, &QPushButton::clicked, [this]{
        QString comment = ui->plainTextEdit->toPlainText(), commentToSend = comment, appealing = appealTo(_recipientNickname);
        if (!_recipientNickname.isEmpty())
        {
            if (commentToSend.startsWith(appealing))
                commentToSend.remove(appealing);
            else
                _recipientCommentId = 0;
        }

        if (!commentToSend.isEmpty())
            RequestManager::instance().postComment(_feedItem->id, commentToSend, _recipientCommentId, [comment, this](bool ok){
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
    if (_deleteItem)
        delete _feedItem;
}

bool CommentsWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o == this && e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Escape)
        {
            close();
            return true;
        }

        if (ke->modifiers() & Qt::ControlModifier)
        {
            switch (ke->key())
            {
            case Qt::Key_Return: case Qt::Key_Enter:
                ui->sendButton->click();
                return true;
            case Qt::Key_R:
                if (ke->modifiers() & Qt::AltModifier)
                {
                    ui->reloadButton->click();
                    return true;
                }
                break;
            default:
                break;
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

void CommentsWidget::showComments(const QSet<quint32> &highlightedComments)
{
    for (auto item : _comments)
    {
        CommentItem *comment = static_cast<CommentItem *>(item);
        auto lwItem = addComment(comment->message, comment->nickname, comment->reputation, comment->recipientNickname);
        if (highlightedComments.contains(comment->id))
        {
            lwItem->setBackgroundColor(Qt::magenta);
            ui->listWidget->scrollToItem(lwItem);
        }
    }
}

QListWidgetItem *CommentsWidget::addComment(const QString &comment, const QString &nickname, qint32 reputation, const QString &recipient)
{
    QString text = QString("%1 %2: ").arg(reputation).arg(nickname);
    if (!recipient.isEmpty())
        text += appealTo(recipient);

    auto item = new QListWidgetItem(text + comment, ui->listWidget);
    if (nickname == RequestManager::instance().userNickname())
        item->setBackgroundColor(Qt::green);
    else if (text.contains(RequestManager::instance().userNickname()))
        item->setBackgroundColor(Qt::cyan);
    return item;
}

QString CommentsWidget::appealTo(const QString &recipient) const
{
    return recipient + ", ";
}
