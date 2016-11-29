#include "commentswidget.h"
#include "ui_commentswidget.h"

#include <QMenu>
#include <QMessageBox>
#include <QClipboard>

static const int MaxCommentLength = 500;

CommentsWidget::CommentsWidget(decltype(FeedItem::id) postId, FeedItem *existingItem, QWidget *parent) : QWidget(parent), ui(new Ui::CommentsWidget), _postId(postId)
{
    ui->setupUi(this);
    ui->listWidget->installEventFilter(this);
    installEventFilter(this);

    if (existingItem)
        ui->messageLabel->setText(QString("%1\n%2 | %3").arg(existingItem->message).arg(existingItem->comments).arg(existingItem->reputation));
    ui->plainTextEdit->setFocus();

    // context menu
    auto upvoteAction = new QAction(tr("Upvote"), this), downvoteAction = new QAction(tr("Downvote"), this), deleteVoteAction = new QAction(tr("Delete vote"), this);

    connect(upvoteAction, &QAction::triggered, [upvoteAction, downvoteAction, deleteVoteAction, this]{
        int row = ui->listWidget->currentRow();
        auto comment = _comments.at(row);
        RequestManager::instance().changeCommentVote(comment->id, false, true, [row, comment, upvoteAction, downvoteAction, deleteVoteAction, this](bool ok){
            if (ok)
            {
                bool existed = _votesHash.find(comment->id) != _votesHash.cend();
                _votesHash[comment->id] = true;
                comment->reputation += existed ? 2 : 1;
                showVoteStatusAtRow(row, true);

                upvoteAction->setEnabled(false);
                downvoteAction->setEnabled(true);
                deleteVoteAction->setEnabled(true);
            }
        });
    });
    ui->listWidget->addAction(upvoteAction);

    connect(downvoteAction, &QAction::triggered, [upvoteAction, downvoteAction, deleteVoteAction, this]{
        int row = ui->listWidget->currentRow();
        auto comment = _comments.at(row);
        RequestManager::instance().changeCommentVote(comment->id, false, false, [row, comment, upvoteAction, downvoteAction, deleteVoteAction, this](bool ok){
            if (ok)
            {
                bool existed = _votesHash.find(comment->id) != _votesHash.cend();
                _votesHash[comment->id] = false;
                comment->reputation -= existed ? 2 : 1;
                showVoteStatusAtRow(row, true);

                upvoteAction->setEnabled(true);
                downvoteAction->setEnabled(false);
                deleteVoteAction->setEnabled(true);
            }
        });
    });
    ui->listWidget->addAction(downvoteAction);

    connect(deleteVoteAction, &QAction::triggered, [upvoteAction, downvoteAction, deleteVoteAction, this]{
        int row = ui->listWidget->currentRow();
        auto comment = _comments.at(row);
        RequestManager::instance().changeCommentVote(comment->id, true, true, [row, comment, upvoteAction, downvoteAction, deleteVoteAction, this](bool ok){
            if (ok)
            {
                bool value = _votesHash[comment->id];
                _votesHash.remove(comment->id);
                value ? --comment->reputation : ++comment->reputation;
                showVoteStatusAtRow(row, true);

                upvoteAction->setEnabled(true);
                downvoteAction->setEnabled(true);
                deleteVoteAction->setEnabled(false);
            }
        });
    });
    ui->listWidget->addAction(deleteVoteAction);

    auto separator = new QAction(this);
    separator->setSeparator(true);
    ui->listWidget->addAction(separator);

    auto copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &CommentsWidget::copyComments);
    ui->listWidget->addAction(copyAction);

    // connects
    connect(ui->listWidget, &QListWidget::doubleClicked, [this](const QModelIndex &index){
        CommentItem *comment = static_cast<CommentItem *>(_comments.at(index.row()));
        if (comment->nickname == RequestManager::nickname)
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
    connect(ui->listWidget, &QListWidget::currentRowChanged, [upvoteAction, downvoteAction, deleteVoteAction, this](int row){
        if (row < 0 || row >= _comments.size())
            return;

        auto comment = static_cast<CommentItem *>(_comments.at(row));
        if (comment->nickname == RequestManager::nickname)
        {
            upvoteAction->setEnabled(false);
            downvoteAction->setEnabled(false);
            deleteVoteAction->setEnabled(false);
            return;
        }

        auto commentId = comment->id;
        auto iter = _votesHash.find(commentId);
        if (iter != _votesHash.cend())
        {
            bool upvoted = iter.value() == 1;
            upvoteAction->setEnabled(!upvoted);
            downvoteAction->setEnabled(upvoted);
            deleteVoteAction->setEnabled(true);
        }
        else
        {
            upvoteAction->setEnabled(true);
            downvoteAction->setEnabled(true);
            deleteVoteAction->setEnabled(false);
        }
    });
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, [this]{
        ui->charactersCountLabel->setText(QString::number(ui->plainTextEdit->toPlainText().size()));
    });
    connect(ui->sendButton, SIGNAL(clicked(bool)), SLOT(sendComment()));
}

CommentsWidget::~CommentsWidget()
{
    delete ui;
    qDeleteAll(_comments);
}

QPlainTextEdit *CommentsWidget::commentTextEdit()
{
    return ui->plainTextEdit;
}

void CommentsWidget::loadNewComments(bool scrollToBottom, const QSet<quint32> &highlightedComments)
{
    RequestManager::instance().requestPostWithId(_postId, [this](FeedItem *feedItem){
        if (feedItem)
            ui->messageLabel->setText(QString("%1\n%2 | %3").arg(feedItem->message).arg(feedItem->comments).arg(feedItem->reputation));
        else
        {
            auto errorStr = tr("Error loading post. Probably it has been deleted.");
            if (ui->messageLabel->text().isEmpty())
                ui->messageLabel->setText(errorStr);
            else if (!_postLoadingError)
                ui->messageLabel->setText(QString("%1\n[%2]").arg(ui->messageLabel->text(), errorStr));
            _postLoadingError = true;
        }
    });

    loadComments([scrollToBottom, highlightedComments, this](const TextItemList &newComments){
        qDeleteAll(_comments);
        _comments = newComments;

        ui->listWidget->clear();
        showComments(highlightedComments);
        if (scrollToBottom)
            ui->listWidget->scrollToBottom();

        requestCommentsVotes(newComments);
    });
}

void CommentsWidget::loadOldComments()
{
    if (_comments.isEmpty())
        return;
    loadComments([this](const TextItemList &newComments){
        _comments = newComments + _comments;

        ui->listWidget->clear();
        showComments();
        ui->listWidget->scrollToTop();

        requestCommentsVotes(newComments);
    }, _comments.first()->id);
}

void CommentsWidget::sendComment()
{
    // toPlainText() replaces non-breaking space with normal space, so we must do the same to stay consistent
    auto comment = ui->plainTextEdit->toPlainText(), commentToSend = comment, appealing = appealTo(_recipientNickname).replace(QChar::Nbsp, QLatin1Char(' '));
    if (!_recipientNickname.isEmpty())
    {
        if (commentToSend.startsWith(appealing))
            commentToSend.remove(appealing);
        else
            _recipientCommentId = 0;
    }
    if (commentToSend.length() > MaxCommentLength)
    {
        QMessageBox::critical(this, QString(), tr("Comment can't be longer than %1 characters.\nPlease split it into multiple.").arg(MaxCommentLength));
        return;
    }

    if (commentToSend.isEmpty())
        return;

    ui->sendButton->setDisabled(true);
    RequestManager::instance().postComment(_postId, commentToSend, _recipientCommentId, [comment, this](bool ok){
        if (ok)
        {
            addComment(comment);
            ui->plainTextEdit->clear();

            _recipientCommentId = 0;
            _recipientNickname.clear();
        }
        else
            QMessageBox::critical(this, QString(), tr("Error sending comment"));
        ui->sendButton->setEnabled(true);
    });
}

void CommentsWidget::copyComments()
{
    auto indexes = ui->listWidget->selectionModel()->selectedRows();
    std::sort(indexes.begin(), indexes.end());

    QStringList comments;
    for (auto index : indexes)
        comments << ui->listWidget->item(index.row())->text();
    qApp->clipboard()->setText(comments.join(QChar::LineFeed));
}

void CommentsWidget::loadComments(std::function<void(const TextItemList &newComments)> callback, decltype(FeedItem::id) commentIdForOlderPosts)
{
    RequestManager::instance().requestComments(_postId, [callback, this](const TextItemList &newComments) {
        if (!newComments.isEmpty())
            callback(newComments);
    }, commentIdForOlderPosts);
}

bool CommentsWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (o == this)
        {
            if (ke->modifiers() & Qt::ControlModifier)
            {
                switch (ke->key())
                {
                case Qt::Key_Return: case Qt::Key_Enter:
                    sendComment();
                    return true;
                }
            }
        }
        else if (o == ui->listWidget)
        {
            if (ke->matches(QKeySequence::Copy))
            {
                copyComments();
                return true;
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

void CommentsWidget::showComments(const QSet<quint32> &highlightedComments)
{
    for (int i = 0; i < _comments.size(); ++i)
    {
        CommentItem *comment = static_cast<CommentItem *>(_comments.at(i));
        auto lwItem = addComment(comment->message, comment->nickname, comment->reputation, comment->recipientNickname, QDateTime::fromTime_t(comment->timestamp));
        showVoteStatusAtRow(i);

        if (highlightedComments.contains(comment->id))
        {
            lwItem->setBackgroundColor(Qt::magenta);
            ui->listWidget->scrollToItem(lwItem);
        }
    }
}

QListWidgetItem *CommentsWidget::addComment(const QString &comment, const QString &nickname, qint32 reputation, const QString &recipient, const QDateTime &dateTime)
{
    auto today = QDate::currentDate(), date = dateTime.date();
    auto daysAgo = date.daysTo(today);
    QLatin1String dateFormat;
    if (daysAgo == 0 || daysAgo == 1)
        dateFormat = QLatin1String("h:mm");
    else if (date.year() == today.year())
        dateFormat = QLatin1String("d.M h:mm");
    else
        dateFormat = QLatin1String("d.M.yy h:mm");

    auto dateTimeStr = dateTime.toString(dateFormat);
    if (daysAgo == 1)
        dateTimeStr = tr("yesterday %1").arg(dateTimeStr);
    auto text = QString("(%1) [%2] %3: ").arg(reputation).arg(dateTimeStr, nickname);
    if (!recipient.isEmpty())
        text += appealTo(recipient);

    auto item = new QListWidgetItem(text + comment, ui->listWidget);
    if (nickname == RequestManager::nickname)
        item->setBackgroundColor(Qt::green);
    else if (text.contains(RequestManager::nickname))
        item->setBackgroundColor(Qt::cyan);
    return item;
}

QString CommentsWidget::appealTo(const QString &recipient) const
{
    return recipient + ", ";
}

void CommentsWidget::showVoteStatusAtRow(int row, bool rewriteVote)
{
    auto comment = _comments.at(row);
    auto item = ui->listWidget->item(row);
    auto text = item->text();
    if (rewriteVote)
        text.remove(0, text.indexOf(')')).prepend(QString("(%1").arg(comment->reputation));

    auto iter = _votesHash.find(comment->id);
    if (iter != _votesHash.cend())
    {
        auto sign = iter.value() == 1 ? "+" : "-";
        if (!text.startsWith(sign))
            text.prepend(sign);
    }

    item->setText(text);
}

void CommentsWidget::requestCommentsVotes(const TextItemList &comments)
{
    QList<decltype(CommentItem::id)> ids;
    for (auto comment : comments)
        ids << comment->id;
    RequestManager::instance().requestCommentsVotes(ids, [this](const QHash<decltype(CommentItem::id), bool> &votesHash){
        _votesHash.unite(votesHash);
        for (int i = 0; i < _comments.size(); ++i)
            showVoteStatusAtRow(i);
    });
}
