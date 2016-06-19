#ifndef COMMENTSWIDGET_H
#define COMMENTSWIDGET_H

#include <QDialog>
#include <QSet>
#include <QDateTime>

#include "feeditem.h"
#include "requestmanager.h"

namespace Ui {
class CommentsWidget;
}
class QListWidgetItem;
class QPlainTextEdit;

class CommentsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommentsWidget(decltype(FeedItem::id) postId, FeedItem *existingItem = nullptr, QWidget *parent = nullptr);
    ~CommentsWidget();

    QPlainTextEdit *commentTextEdit();
    decltype(FeedItem::id) postId() const { return _postId; }

    void loadNewComments(bool scrollToBottom, const QSet<quint32> &highlightedComments = QSet<quint32>());
    void loadOldComments();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void sendComment();
    void copyComments();

private:
    Ui::CommentsWidget *ui;
    decltype(FeedItem::id) _postId;
    TextItemList _comments;
    decltype(CommentItem::id) _recipientCommentId = 0;
    QString _recipientNickname;
    QHash<decltype(CommentItem::id), bool> _votesHash;
    bool _postLoadingError = false;

    void loadComments(std::function<void(const TextItemList &newComments)> callback, decltype(CommentItem::id) commentIdForOlderPosts = 0);
    void showComments(const QSet<quint32> &highlightedComments = QSet<quint32>());
    QListWidgetItem *addComment(const QString &comment, const QString &nickname = RequestManager::nickname, qint32 reputation = 0, const QString &recipient = QString(), const QDateTime &dateTime = QDateTime::currentDateTime());
    QString appealTo(const QString &recipient) const;
    void showVoteStatusAtRow(int row, bool rewriteVote = false);
    void requestCommentsVotes(const TextItemList &comments);
};

#endif // COMMENTSWIDGET_H
