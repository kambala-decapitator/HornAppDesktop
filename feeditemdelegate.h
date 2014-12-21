#ifndef FEEDITEMDELEGATE_H
#define FEEDITEMDELEGATE_H

#include <QStyledItemDelegate>

struct FeedItem;

class FeedItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FeedItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

private:
    FeedItem *itemAtIndex(const QModelIndex &index) const;
};

#endif // FEEDITEMDELEGATE_H
