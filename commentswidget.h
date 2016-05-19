#ifndef COMMENTSWIDGET_H
#define COMMENTSWIDGET_H

#include <QWidget>
#include <QSet>

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

    void showComments(const TextItemList &comments, const QSet<quint32> &highlightedComments = QSet<quint32>());
    QListWidgetItem *addComment(const QString &comment, const QString &nickname = RequestManager::instance().userNickname(), qint32 reputation = 0, const QString &recipient = QString());
    QString appealTo(const QString &recipient) const;
};

#endif // COMMENTSWIDGET_H
