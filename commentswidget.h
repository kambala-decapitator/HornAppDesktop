#ifndef COMMENTSWIDGET_H
#define COMMENTSWIDGET_H

#include <QWidget>
#include <QSet>
#include <QDateTime>

#include "feeditem.h"
#include "requestmanager.h"

namespace Ui {
class CommentsWidget;
}
class QListWidgetItem;

class CommentsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommentsWidget(FeedItem *feedItem, const TextItemList &comments, bool deleteItem, const QSet<quint32> &highlightedComments, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~CommentsWidget();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    Ui::CommentsWidget *ui;
    TextItemList _comments;
    quint32 _recipientCommentId;
    QString _recipientNickname;
    FeedItem *_feedItem;
    bool _deleteItem;
    QHash<decltype(CommentItem::id), bool> _votesHash;

    void showComments(const QSet<quint32> &highlightedComments = QSet<quint32>());
    QListWidgetItem *addComment(const QString &comment, const QString &nickname = RequestManager::nickname, qint32 reputation = 0, const QString &recipient = QString(), const QDateTime &dateTime = QDateTime::currentDateTime());
    QString appealTo(const QString &recipient) const;
    void showVoteStatusAtRow(int row, bool rewriteVote = false);
    void requestCommentsVotes(const TextItemList &comments);
};

#endif // COMMENTSWIDGET_H
