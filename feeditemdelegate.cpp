#include "feeditemdelegate.h"
#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"
#include "feedlistmodel.h"

#include <QPainter>

FeedItemDelegate::FeedItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void FeedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    painter->drawText(option.rect, Qt::TextWordWrap | Qt::AlignHCenter | Qt::AlignVCenter, index.data().toString());
//    painter->drawRect(option.rect);ß
    QPaintDevice *original_pdev_ptr = painter->device();
    FeedItemWidget w;
    QTextOption opt = w.ui->plainTextEdit->document()->defaultTextOption();
    opt.setAlignment(Qt::AlignCenter);
    w.ui->plainTextEdit->document()->setDefaultTextOption(opt);
    FeedItem *item = static_cast<const FeedListModel *>(index.model())->itemAtModelIndex(index);
    w.ui->plainTextEdit->setPlainText(item->message);
    w.ui->dateTimeEdit->setDateTime(QDateTime::fromTime_t(item->timestamp));
    w.ui->lcdNumber->display(item->reputation);
    w.setGeometry(option.rect);
//    w.adjustSize();
    painter->end();
    w.render(painter->device(), option.rect.topLeft(), QRegion(QRect(QPoint(), option.rect.size())), QWidget::RenderFlag::DrawChildren);
    painter->begin(original_pdev_ptr);
}
