#include "feeditemdelegate.h"

#include <QPainter>

FeedItemDelegate::FeedItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void FeedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->drawText(option.rect, Qt::TextWordWrap | Qt::AlignHCenter | Qt::AlignVCenter, index.data().toString());
    painter->drawRect(option.rect);
}
