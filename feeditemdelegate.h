#ifndef FEEDITEMDELEGATE_H
#define FEEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class FeedItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FeedItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:

};

#endif // FEEDITEMDELEGATE_H
