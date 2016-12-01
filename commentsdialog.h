#ifndef COMMENTSDIALOG_H
#define COMMENTSDIALOG_H

#include <QDialog>
#include <QSet>
#include "feeditem.h"

namespace Ui {
class CommentsDialog;
}

class CommentsDialog : public QDialog
{
    Q_OBJECT

public:
    static CommentsDialog &instance()
    {
        static CommentsDialog obj;
        return obj;
    }

    void showComments(decltype(TextItem::id) postId, FeedItem *existingItem = nullptr, const QSet<quint32> &highlightedComments = QSet<quint32>());

public slots:
    void reject();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void closeTab(int index);

private:
    Ui::CommentsDialog *ui;

    explicit CommentsDialog();
    virtual ~CommentsDialog();
};

#endif // COMMENTSDIALOG_H
